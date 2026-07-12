#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "cbor2>=6.1.3",
#     "click>=8.4.2",
#     "cryptography>=49.0.0",
#     "pycose>=1.1.0",
#     "pyserial>=3.5",
#     "pyyaml>=6.0.3",
#     "requests>=2.34.2",
# ]
# ///

import base64
from enum import Enum
import json
import os
from pathlib import Path
import random
import sys
from time import sleep
from typing import List, NoReturn, TypedDict

import serial

import cbor2
import click
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey, Ed25519PublicKey
import pycose
from pycose.keys import CoseKey
from pycose.messages import Sign1Message
import requests
import yaml


# Helper functions and classes.
class DeviceType(Enum):
    """KID device type."""
    gate = 0
    mate = 1
    server = 2
    root = 3

class SupportedNodeType(Enum):
    """Node type supported by this script."""
    senseGate = DeviceType.gate.value
    senseMate = DeviceType.mate.value

class TTNConfig(TypedDict):
    """TTN config structure."""
    instance: str
    applicationID: str
    authToken: str

class Config(TypedDict):
    """Config structure."""
    ttn: TTNConfig

class BaseIdentityInformation(TypedDict):
    """Base identity information structure shared by all identity information variants."""
    kid: bytes
    privateKey: Ed25519PrivateKey
    publicKey: Ed25519PublicKey

class RootIdentityInformation(BaseIdentityInformation):
    """Root identity information."""

class SignedPublicIdentityIDInfo(TypedDict):
    """Signed public identity structure. For use in identity information."""
    publicIdentity: bytes
    signature: bytes

class TTNIDInfo(TypedDict):
    """TTN structure. For use in identity information."""
    joinEUI: str
    devEUI: str
    appKey: str

class NodeIdentityInformation(BaseIdentityInformation):
    """Node identity information."""
    signedPublicIdentity: SignedPublicIdentityIDInfo
    rootKey: Ed25519PublicKey
    ttn: TTNIDInfo
    provisioningPayload: str

class IdentityEncoder(json.JSONEncoder):
    """JSONEncoder with special handling for identity information dictionaries. Taking care of having all data nicely encode."""
    def default(self, o):
        # Handle bytes (used for the roots and nodes kids and cbor data).
        if isinstance(o, bytes):
            return o.hex()
        # Handle Ed25519PrivateKey (used for the roots and nodes private keys).
        if isinstance(o, Ed25519PrivateKey):
            priv_key_str = o.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            ).decode("utf-8")
            return priv_key_str
        # Handle Ed25519PublicKey (used for the roots and nodes public keys, as well as the nodes rootKey).
        if isinstance(o, Ed25519PublicKey):
            pub_key_str = o.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            ).decode("utf-8")
            return pub_key_str
        return super().default(o)
    
class IdentityDecoder(json.JSONDecoder):
    """JSONDecoder with special handling for identity information dictionaries. Taking care of cleanly undoing the conversions done by IdentityEncoder."""
    def __init__(self, **kwargs):
        kwargs["object_hook"] = self.object_hook
        super().__init__(**kwargs)
    
    def object_hook(self, obj):
        # Handle the kid stored as a hex string and convert it back to bytes.
        if "kid" in obj and isinstance(obj["kid"], str):
            try:
                obj["kid"] = bytes.fromhex(obj["kid"])
            except:
                raise Exception("Couldn't create bytes from provided value for kid.")

            expected_kid_len = 4 
            kid_len = len(obj["kid"])
            if kid_len != expected_kid_len:
                raise Exception(f"Expected length {expected_kid_len} for kid, got {kid_len}.")
        # Handle the publicIdentity, signature and provisioningPayload stored as hex strings and convert them back to bytes.
        for hex_data_key in ["publicIdentity", "signature"]:
            if hex_data_key in obj and isinstance(obj[hex_data_key], str):
                try:
                    obj[hex_data_key] = bytes.fromhex(obj[hex_data_key])
                except:
                    raise Exception(f"Couldn't create bytes from provided value for {hex_data_key}.")
        # Handle the privateKey stored in PKCS8 and PEM and create an Ed25519PrivateKey from it.
        if "privateKey" in obj and isinstance(obj["privateKey"], str):
            try:
                obj["privateKey"] = serialization.load_pem_private_key(
                    str.encode(obj["privateKey"]),
                    password=None,
                )
            except:
                raise Exception("Couldn't create Ed25519PrivateKey from provided value for privateKey.")
        # Handle the publicKey stored in PEM and create an Ed25519PublicKey from it.
        if "publicKey" in obj and isinstance(obj["publicKey"], str):
            try:
                obj["publicKey"] = serialization.load_pem_public_key(
                    str.encode(obj["publicKey"]),
                )
            except:
                raise Exception("Couldn't create Ed25519PublicKey from provided value for publicKey.")
        # Handle the rootKey stored in PEM and create an Ed25519PublicKey from it.
        if "rootKey" in obj and isinstance(obj["rootKey"], str):
            try:
                obj["rootKey"] = serialization.load_pem_public_key(
                    str.encode(obj["rootKey"]),
                )
            except:
                raise Exception("Couldn't create Ed25519PublicKey from provided value for rootKey.")
        return obj

def print_error(msg: str) -> NoReturn:
    """Prints an error message."""
    click.secho(msg, err=True, fg="red")

def print_info(msg: str) -> NoReturn:
    """Prints an info message."""
    click.secho(msg, err=True, fg="blue")

def get_base_dir() -> str:
    """Gets the path of the base directory, which is the directory this script file is in."""
    return f"{os.path.dirname(os.path.realpath(__file__))}"

def get_config() -> Config:
    """"Loads the configuration, validates it and sets default values, if necessary."""
    # Ensure config exists and can be loaded.
    config_path = f"{base_dir}/config.yaml"
    if not Path(config_path).exists():
        raise Exception(f"No configuration file ({config_path}) exists.")
    with open(config_path, "r") as f:
        config: Config = yaml.safe_load(f)
    if config is None:
        raise Exception("Couldn't load configuration.")

    # Validate config and set default values, if necessary.
    if "ttn" not in config:
        raise Exception("No ttn config in configuration.")
    if not isinstance(config["ttn"], dict):
        raise Exception("ttn config isn't of expected format.")
    
    if "instance" not in config["ttn"]:
        # Default to "eu1.cloud.thethings.network" for TTN instance.
        config["ttn"]["instance"] = "eu1.cloud.thethings.network"
    if not isinstance(config["ttn"]["instance"], str):
        raise Exception("ttn.instance isn't of expected format.")
    
    if "applicationID" not in config["ttn"]:
        raise Exception("ttn.applicationID missing in configuration.")
    if not isinstance(config["ttn"]["applicationID"], str):
        raise Exception("ttn.applicationID isn't of expected format.")

    if "authToken" not in config["ttn"]:
        raise Exception("ttn.authToken missing in configuration.")
    if not isinstance(config["ttn"]["authToken"], str):
        raise Exception("ttn.authToken isn't of expected format.")
    
    return config

def create_kid(device_type: DeviceType, device_id: int) -> bytes:
    """Creates a kid from a device type and device id."""
    return bytes([
        0,
        0,
        device_type.value,
        device_id
    ])

def kid_to_filename(kid: bytes) -> str:
    """Creates an identity information file name from a kid."""
    if kid[2] == DeviceType.gate.value:
        device_type_str = "senseGate"
        file_type = "node_id_info"
    elif kid[2] == DeviceType.mate.value:
        device_type_str = "senseMate"
        file_type = "node_id_info"
    elif kid[2] == DeviceType.server.value:
        raise Exception(f"Device type {DeviceType.server.name} not supported.")
    elif kid[2] == DeviceType.root.value:
        device_type_str = "root"
        file_type = "root_id_info"
    else:
        raise Exception(f"Device type {kid[2]} not implemented.")
    
    return f"{device_type_str}_{kid[3]:03d}.{file_type}.json"

def kid_to_str(kid: bytes) -> str:
    """Nicely formats a kid as a string."""
    return f"{kid[0]:#04x} {kid[1]:#04x} {kid[2]:#04x} {kid[3]:#04x}"

def validate_base_identity_information(id_info: BaseIdentityInformation):
    """Validates the given identity information to be valid BaseIdentityInformation."""
    if "kid" not in id_info:
        raise Exception("kid not in id info.")
    if not isinstance(id_info["kid"], bytes):
        raise Exception("kid isn't of expected format (bytes).")
    if len(id_info["kid"]) != 4:
        raise Exception("kid should have a size of 4.")

    if "privateKey" not in id_info:
        raise Exception("privateKey not in id info.")
    if not isinstance(id_info["privateKey"], Ed25519PrivateKey):
        raise Exception("privateKey isn't of expected format (Ed25519PrivateKey).")

    if "publicKey" not in id_info:
        raise Exception("publicKey not in id info.")
    if not isinstance(id_info["publicKey"], Ed25519PublicKey):
        raise Exception("publicKey isn't of expected format (Ed25519PublicKey).")

    if id_info["privateKey"].public_key().public_bytes_raw() != id_info["publicKey"].public_bytes_raw():
        raise Exception("publicKey in id info doesn't match public key derived from privateKey.")

def validate_node_identity_information(id_info: NodeIdentityInformation):
    """Validates the given node identity information to be valid NodeIdentityInformation."""
    validate_base_identity_information(id_info)

    if "signedPublicIdentity" not in id_info:
        raise Exception("signedPublicIdentity not in id info.")
    if not isinstance(id_info["signedPublicIdentity"], dict):
        raise Exception("signedPublicIdentity isn't of expected format (dict).")

    signed_public_identity_attrs = ["publicIdentity", "signature"]
    for attr in signed_public_identity_attrs:
        if attr not in id_info["signedPublicIdentity"]:
            raise Exception(f"signedPublicIdentity.{attr} not in id info.")
        if not isinstance(id_info["signedPublicIdentity"][attr], bytes):
            raise Exception(f"signedPublicIdentity.{attr} isn't of expected format (bytes).")

    if "rootKey" not in id_info:
        raise Exception("rootKey not in id info.")
    if not isinstance(id_info["rootKey"], Ed25519PublicKey):
        raise Exception("rootKey isn't of expected format (Ed25519PublicKey).")

    if "ttn" not in id_info:
        raise Exception("ttn not in id info.")
    if not isinstance(id_info["ttn"], dict):
        raise Exception("ttn isn't of expected format (dict).")

    # Attributes and their expected lengths.
    ttn_attrs = {
        "joinEUI": 16,
        "devEUI": 16,
        "appKey": 32
    }
    for attr, attr_len in ttn_attrs.items():
        if attr not in id_info["ttn"]:
            raise Exception(f"ttn.{attr} not in id info.")
        if not isinstance(id_info["ttn"][attr], str):
            raise Exception(f"ttn.{attr} isn't of expected format (str).")
        if len(id_info["ttn"][attr]) != attr_len:
            raise Exception(f"ttn.{attr} should have a size of 16.")
        
    if "provisioningPayload" not in id_info:
        raise Exception("provisioningPayload not in id info.")
    if not isinstance(id_info["provisioningPayload"], str):
        raise Exception("provisioningPayload isn't of expected format (str).")
    try:
        base64.b64decode(
            id_info["provisioningPayload"],
            validate=True
        )
    except Exception as e:
        raise Exception(f"Error trying to decode provisioningPayload as base64: {e}")

def load_root_identity_info() -> RootIdentityInformation:
    """Loads the root identity info for id 0."""
    id = 0
    kid = create_kid(DeviceType.root, id)

    # Where to save the identity information.
    root_id_info_dir = f"{identity_dir}/root"
    root_id_info_file_name = kid_to_filename(kid)
    root_id_info_path = f"{root_id_info_dir}/{root_id_info_file_name}"

    # Check if the id exists.
    if not Path(root_id_info_path).exists():
        raise Exception(f"Root identity with ID {id} doesn't exist.")

    with open(root_id_info_path) as f:
        root_id_info = json.load(
            f,
            cls=IdentityDecoder,
        )

    try:
        validate_base_identity_information(root_id_info)
    except Exception as e:
        raise Exception(f"Error validating ID info: {e}")

    return root_id_info

def kid_to_ttn_device_id(kid: bytes) -> str:
    """Creates a device ID for use in TTN from a kid."""
    if kid[2] == DeviceType.gate.value:
        device_type_str = "sensegate"
    elif kid[2] == DeviceType.mate.value:
        device_type_str = "sensemate"
    elif (kid[2] == DeviceType.server.value) or (kid[2] == DeviceType.root.value):
        raise Exception(f"Device type {DeviceType.server.name} not supported.")
    else:
        raise Exception(f"Device type {kid[2]} not implemented.")
    
    return f"{device_type_str}-{kid[3]:03d}"

def ttn_get_api_base() -> str:
    return f"https://{config["ttn"]["instance"]}/api/v3"

def ttn_get_headers_with_auth() -> dict:
    return {
        "Authorization": f"Bearer {config["ttn"]["authToken"]}",
        "Content-Type": "application/json"
    }

def ttn_check_device_exists(ttn_device_id: str) -> bool:
    """Checks if the specified device exists in TTN. Returns true, if it does, false, if not."""
    url = f"{ttn_get_api_base()}/applications/{config["ttn"]["applicationID"]}/devices/{ttn_device_id}"
    response = requests.get(
        url,
        headers=ttn_get_headers_with_auth()
    )

    if response.status_code == 200:
        return True
    elif response.status_code == 404:
        return False
    else:
        raise Exception(f"Received unexpected status code checking if device ({ttn_device_id}) exists: {response.status_code}\n{response.text}")

def ttn_delete_device(ttn_device_id: str) -> NoReturn:
    # https://www.thethingsindustries.com/docs/api/reference/grpc/end_device/
    # Delete the device from the Identity Server (IS), Join Server (JS), Network Server (NS) and Application Server (AS).
    # Order AS -> NS -> JS -> IS recommended.

    # First delete from the AS, NS and JS.
    for server in ["js", "ns", "as"]:
        url = f"{ttn_get_api_base()}/{server}/applications/{config["ttn"]["applicationID"]}/devices/{ttn_device_id}"
        response = requests.delete(
            url,
            headers=ttn_get_headers_with_auth()
        )
        if response.status_code not in [200, 404]:
            raise Exception(f"Received unexpected status code while trying to delete device ({ttn_device_id}) from TTN {server}: {response.status_code}\n{response.text}")
    
    # Then delete from the IS.
    url = f"{ttn_get_api_base()}/applications/{config["ttn"]["applicationID"]}/devices/{ttn_device_id}"
    response = requests.delete(
        url,
        headers=ttn_get_headers_with_auth()
    )
    if response.status_code not in [200, 404]:
        raise Exception(f"Received unexpected status code while trying to delete device ({ttn_device_id}) from TTN Identity Server: {response.status_code}\n{response.text}")

def ttn_create_device(ttn_device_id: str, ttn_join_eui: str, ttn_dev_eui: str, ttn_app_key: str) -> NoReturn:
    # https://www.thethingsindustries.com/docs/api/reference/grpc/end_device/
    # Add the device to the Identity Server (IS), Join Server (JS), Network Server (NS) and Application Server (AS).

    # First Identity Server.
    url = f"{ttn_get_api_base()}/applications/{config["ttn"]["applicationID"]}/devices"
    data = {
        "end_device": {
            "ids": {
                "device_id": ttn_device_id,
                "dev_eui": ttn_dev_eui,
                "join_eui": ttn_join_eui,
                "application_ids": {
                    "application_id": config["ttn"]["applicationID"]
                }
            },
            "name": ttn_device_id,
            "description": "Created by identity-manager.py script.",
            "supports_join": True,
            "lorawan_version": "1.0.3",
            "lorawan_phy_version": "1.0.3-a",
            "network_server_address": config["ttn"]["instance"],
            "application_server_address": config["ttn"]["instance"],
            "join_server_address": config["ttn"]["instance"]
        },
        "field_mask": {
            "paths": [
                "ids",
                "name",
                "description",
                "supports_join",
                "lorawan_version",
                "lorawan_phy_version",
                "network_server_address",
                "application_server_address",
                "join_server_address"
            ]
        }
    }
    response = requests.post(
        url,
        headers=ttn_get_headers_with_auth(),
        json=data
    )
    if response.status_code != 200:
        raise Exception(f"Received unexpected status code while trying to create device ({ttn_device_id}) in TTN Identity Server: {response.status_code}\n{response.text}")

    # Then Join Server.
    url = f"{ttn_get_api_base()}/js/applications/{config["ttn"]["applicationID"]}/devices/{ttn_device_id}"
    data = {
        "end_device": {
            "ids": {
                "device_id": ttn_device_id,
                "dev_eui": ttn_dev_eui,
                "join_eui": ttn_join_eui
            },
            "root_keys": {
                "app_key": {
                    "key": ttn_app_key
                }
            }
        },
        "field_mask": {
            "paths": [
                "ids",
                "root_keys"
            ]
        }
    }
    response = requests.put(
        url,
        headers=ttn_get_headers_with_auth(),
        json=data
    )
    if response.status_code != 200:
        raise Exception(f"Received unexpected status code while trying to create device ({ttn_device_id}) in TTN Join Server: {response.status_code}\n{response.text}")

    # Then Network Server.
    url = f"{ttn_get_api_base()}/ns/applications/{config["ttn"]["applicationID"]}/devices/{ttn_device_id}"
    data = {
        "end_device": {
            "ids": {
                "device_id": ttn_device_id,
                "dev_eui": ttn_dev_eui,
                "join_eui": ttn_join_eui
            },
            "supports_join": True,
            "lorawan_version": "1.0.3",
            "lorawan_phy_version": "1.0.3-a",
            "frequency_plan_id": "EU_863_870",
            "mac_settings": {
                "resets_f_cnt": True
            },
        },
        "field_mask": {
            "paths": [
                "ids",
                "supports_join",
                "lorawan_version",
                "lorawan_phy_version",
                "frequency_plan_id",
                "mac_settings.resets_f_cnt",
            ]
        }
    }
    response = requests.put(
        url,
        headers=ttn_get_headers_with_auth(),
        json=data
    )
    if response.status_code != 200:
        raise Exception(f"Received unexpected status code while trying to create device ({ttn_device_id}) in TTN Network Server: {response.status_code}\n{response.text}")

    # Finally Application Server.
    url = f"{ttn_get_api_base()}/as/applications/{config["ttn"]['applicationID']}/devices/{ttn_device_id}"
    data = {
        "end_device": {
            "ids": {
                "device_id": ttn_device_id
            },
        },
        "field_mask": {
            "paths": [
                "ids"
            ]
        }
    }
    response = requests.put(
        url,
        headers=ttn_get_headers_with_auth(),
        json=data
    )
    if response.status_code != 200:
        raise Exception(f"Received unexpected status code while trying to create device ({ttn_device_id}) in TTN Application Server: {response.status_code}\n{response.text}")


# CLI
@click.group()
def identity_manager():
    global base_dir
    global identity_dir
    global config

    # Set directories.
    base_dir = get_base_dir()
    identity_dir = f"{base_dir}/identities"

    # Load the config file.
    try:
        config = get_config()
    except Exception as e:
        print_error("[!] Error loading configuration:")
        print_error(f"[!] {e}")
        print_error("[!] Exiting.")
        sys.exit(1)

@identity_manager.group()
def root():
    """Root identity management."""
    pass

@root.command()
@click.option("-f", "--force", is_flag=True, help="Forcefully create the identity, even if one with the same ID already exists.")
def create(force):
    """Create a root identity."""

    # Only support a single root identity with id 0 for now.
    root_id = 0
    root_kid = create_kid(DeviceType.root, root_id)

    # Where to save the identity information.
    save_dir = f"{identity_dir}/root"
    save_file_name = kid_to_filename(root_kid)
    save_path = f"{save_dir}/{save_file_name}"

    # Check if the root identity information already exists.
    if Path(save_path).exists() and not force:
        print_error(f"[!] Root identity with ID {root_id} ({save_file_name}) already exists. Exiting.")
        sys.exit(1)

    # Generate the root key.
    root_key = Ed25519PrivateKey.generate()

    # Create the root identity information and save it to disk.
    root_identity_information: RootIdentityInformation = {
        "kid": root_kid,
        "privateKey": root_key,
        "publicKey": root_key.public_key(),
    }
    os.makedirs(save_dir, exist_ok=True)
    with open(save_path, "w") as f:
        f.write(json.dumps(
            root_identity_information,
            cls=IdentityEncoder,
            indent=2
        ))
    
    # Print information about the identity.
    print_info(
        "[i] Successfully created a root identity.\n"
        + f"      id:                 {root_identity_information["kid"][3]}\n"
        + f"      key id:             {kid_to_str(root_identity_information["kid"])}\n"
        + f"      identity info path: {save_path}"
    )

@identity_manager.group()
def node():
    """Node identity management."""
    pass

@node.command()
@click.option("-i", "--device-id", type=click.INT, help="The device ID for which to create the identity information.")
@click.option("-f", "--force", is_flag=True, help="Forcefully create the identity, even if one with the same ID already exists.")
@click.argument("type", type=click.Choice(SupportedNodeType, case_sensitive=False))
def create(device_id, force, type):
    """Create a node identity."""

    # Load the root identity information.
    try:
        root_identity_information = load_root_identity_info()
    except Exception as e:
        print_error("[!] Error loading root identity information:")
        print_error(f"[!] {e}")
        print_error("[!] Exiting.")
        sys.exit(1)

    # Where to save the node identity information.
    save_dir = f"{identity_dir}/node"

    if device_id is not None:
        # Use the given device ID and check if it's valid.
        if device_id < 0:
            print_error("[!] Device ID must be a positive integer. Exiting.")
            sys.exit(1)
        if device_id > 255:
            print_error("[!] Device ID can't be greater than 255 (hex: ff). Exiting.")
            sys.exit(1)

        node_kid = create_kid(type, device_id)
        save_file_name = kid_to_filename(node_kid)
        save_path = f"{save_dir}/{save_file_name}"
    else:
        # If no device ID was provided, find the next free device ID.
        device_id = 1
        while True:
            if device_id > 255:
                print_error("[!] All device IDs up to and including 255 (hex: ff) are already in use. Consider deleting an old node. Exiting.")
                sys.exit(1)

            node_kid = create_kid(type, device_id)
            save_file_name = kid_to_filename(node_kid)
            save_path = f"{save_dir}/{save_file_name}"

            if not Path(save_path).exists():
                break

            device_id += 1

    # Set the nodes TTN device id.
    node_ttn_device_id = kid_to_ttn_device_id(node_kid)
    
    # Check if the node identity information already exists.
    if Path(save_path).exists() and not force:
        print_error(f"[!] Node identity information for device ID {id} ({save_file_name}) already exists. Exiting.")
        sys.exit(1)
    # Check if node already exists in TTN.
    try:
        ttn_device_exists = ttn_check_device_exists(node_ttn_device_id)
    except Exception as e:
        print_error(f"[!] Error while checking, if device already exists in TTN:")
        print_error(f"[!] {e}")
        print_error("[!] Exiting.")
        sys.exit(1)
    if ttn_device_exists and not force:
        print_error(f"[!] Device with TTN device ID {node_ttn_device_id} already exists in The Things Network. Exiting.")
        sys.exit(1)
    if ttn_device_exists:
        try:
            print_info(f"[!] Deleting existing device with TTN device ID {node_ttn_device_id} from The Things Network...")
            ttn_delete_device(node_ttn_device_id)
        except Exception as e:
            print_error(f"[!] Error while deleting device from The Things Network:")
            print_error(f"[!] {e}")
            print_error("[!] Exiting.")
            sys.exit(1)

    # Generate the node key.
    node_key = Ed25519PrivateKey.generate()

    # Create the nodes signed public identity.
    node_pubid_raw = [
        node_kid,
        node_key.public_key().public_bytes_raw()
    ]
    node_pubid_cbor = cbor2.dumps(node_pubid_raw)
    root_cose_key = CoseKey.from_pem_private_key(
        root_identity_information["privateKey"].private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        ).decode("utf-8")
    )
    cose_sign1_msg = Sign1Message(
        phdr={
            pycose.headers.Algorithm: pycose.algorithms.EdDSA,
            pycose.headers.KID: root_identity_information["kid"]
        }
    )
    cose_sign1_msg.key = root_cose_key
    node_pubid_cbor_cose_sign1_msg_with_signature = cose_sign1_msg.encode(
        detached_payload=node_pubid_cbor
    )

    # Create node in TTN.
    try:
        # https://www.thethingsindustries.com/docs/getting-started/glossary/
        ttn_join_eui = "0000000000000000"  # e.g. für TTI Standard
        ttn_dev_eui = ''.join(random.choice('0123456789ABCDEF') for _ in range(16))
        ttn_app_key = ''.join(random.choice('0123456789ABCDEF') for _ in range(32))

        ttn_create_device(node_ttn_device_id, ttn_join_eui, ttn_dev_eui, ttn_app_key)
    except Exception as e:
        print_error(f"[!] Error while trying to create device in The Things Network:")
        print_error(f"[!] {e}")
        try:
            print_info(f"[!] Cleaning up, trying to delete device from The Things Network...")
            ttn_delete_device(node_ttn_device_id)
        except Exception as e:
            print_error(f"[!] Error while deleting device from The Things Network:")
            print_error(f"[!] {e}")
        print_error("[!] Exiting.")
        sys.exit(1)

    # Create the provisioning payload.
    provisioning_payload_raw = [
        # root public identity:
        root_identity_information["kid"],                          # kid
        root_identity_information["publicKey"].public_bytes_raw(), # public key
        # node private identity:
        node_kid,                                                   # kid
        node_key.private_bytes_raw(),                               # private key
        # node signed public identity:
        node_pubid_cbor,                                            # public identity
        node_pubid_cbor_cose_sign1_msg_with_signature,              # cose sign1 message with signatur0
        # ttn configuration:
        bytes.fromhex(ttn_join_eui),                                # JoinEUI
        bytes.fromhex(ttn_dev_eui),                                 # DevEUI
        bytes.fromhex(ttn_app_key)                                  # AppKey
    ]
    provisioning_payload_cbor = cbor2.dumps(provisioning_payload_raw)
    provisioning_payload_base64_str = base64.b64encode(provisioning_payload_cbor).decode("utf-8")

    # Create the node identity information and save it to disk.
    node_identity_information = {
        "kid": node_kid,
        "privateKey": node_key,
        "publicKey": node_key.public_key(),
        "signedPublicIdentity": {
            "publicIdentity": node_pubid_cbor,
            "signature": node_pubid_cbor_cose_sign1_msg_with_signature
        },
        "rootKey": root_identity_information["publicKey"],
        "ttn": {
            "joinEUI": ttn_join_eui,
            "devEUI": ttn_dev_eui,
            "appKey": ttn_app_key
        },
        "provisioningPayload": provisioning_payload_base64_str
    }
    os.makedirs(save_dir, exist_ok=True)
    with open(save_path, "w") as f:
        f.write(json.dumps(
            node_identity_information,
            cls=IdentityEncoder,
            indent=2
        ))

    # Print information about the identity.
    print_info(
        "[i] Successfully created device in The Things Network.\n"
        + f"      JoinEUI: {ttn_join_eui}\n"
        + f"      DevEUI:  {ttn_dev_eui}\n"
        + f"      AppKey:  {ttn_app_key}\n"
    )
    print_info(
        "[i] Successfully created a node identity.\n"
        + f"      id:                 {node_identity_information["kid"][3]:03d}\n"
        + f"      key id:             {kid_to_str(node_identity_information["kid"])}\n"
        + f"      identity info path: {save_path}\n"
    )
    print_info(
        "[i] Node provisioning payload:\n"
        + node_identity_information["provisioningPayload"]
    )

@node.command()
@click.argument("type", type=click.Choice(SupportedNodeType, case_sensitive=False))
@click.argument("device_id", type=click.INT)
def provision(type, device_id):
    """Sends the provision payload of the specified node via serial."""

    kid = create_kid(type, device_id)

    # Where to load the node identity information from.
    node_id_info_dir = f"{identity_dir}/node"
    node_id_info_file_name = kid_to_filename(kid)
    node_id_info_path = f"{node_id_info_dir}/{node_id_info_file_name}"

    # Load the node identity information.
    if not Path(node_id_info_path).exists():
        raise Exception(f"Node identity with ID {device_id} doesn't exist.")
    with open(node_id_info_path) as f:
        node_id_info: NodeIdentityInformation = json.load(
            f,
            cls=IdentityDecoder,
        )
    try:
        validate_node_identity_information(node_id_info)
    except Exception as e:
        print_error(f"[!] Error while validating the node identity information at {node_id_info_path}:")
        print_error(f"[!] {e}")
        print_error("[!] Exiting.")

    # Send the payload using serial.
    with serial.Serial(port="/dev/ttyACM0", baudrate=115200) as ser:
        ser.write(
            data="\n".encode()
        )
        # Call the provision function.
        print_info(f"[i] Sending command: provision_own_identity")
        ser.write(
            data="provision_own_identity\n".encode()
        )
        sleep(1)
        # Send the payload.
        for i in range(0, len(node_id_info["provisioningPayload"]), 128):
            to_write = node_id_info["provisioningPayload"][i:i+128]
            print_info(f"[i] Sending data: {to_write}")
            res = ser.write(
                data=to_write.encode()
            )
            print_info(f"[i] Written {res} bytes.")
        sleep(1)
        # Reboot the node.
        print_info(f"[i] Rebooting...")
        ser.write(
            data="\nreboot\n".encode()
        )
        print_info(f"[i] Node provisioned.")

@node.command()
def wipe():
    """Wipes the connected node via serial."""

    with serial.Serial(port="/dev/ttyACM0", baudrate=115200) as ser:
        ser.write(
            data="\n".encode()
        )
        # Wipe the node.
        print_info(f"[i] Sending command: wipe")
        ser.write(
            data="wipe\n".encode()
        )
        sleep(1)
        # Reboot.
        print_info(f"[i] Rebooting...")
        ser.write(
            data="\nreboot\n".encode()
        )
        print_info(f"[i] Node wiped.")

if __name__ == "__main__":
    identity_manager()
