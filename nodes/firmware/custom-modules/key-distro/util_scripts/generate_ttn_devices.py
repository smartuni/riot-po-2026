import os
import requests
import random
import yaml
from pathlib import Path

with open("config.yaml", "r") as f:
    config = yaml.safe_load(f)

tti_instance = config["tti_instance"]

sensemate_count = config["sensemates"]["count"]
sensemate_prefix = config["sensemates"]["id_prefix"]

sensegate_count = config["sensegates"]["count"]
sensegate_prefix = config["sensegates"]["id_prefix"]

# Beispiel Device-IDs erstellen
sensemate_ids = [f"{sensemate_prefix}-{i+1}" for i in range(sensemate_count)]
sensegate_ids = [f"{sensegate_prefix}-{i+1}" for i in range(sensegate_count)]

# Konfiguration
TTI_API_BASE = f"https://{tti_instance}/api/v3"
APPLICATION_ID = config["application_id"]
with open("secrets.yaml", "r") as f:
    secrets = yaml.safe_load(f)

AUTH_TOKEN = secrets["ttn_auth_token"]
headers = {
    "Authorization": f"Bearer {AUTH_TOKEN}",
    "Content-Type": "application/json",
}

# --- Hilfsfunktionen ---
def generate_random_eui():
    return ''.join(random.choice('0123456789ABCDEF') for _ in range(16))

def generate_random_key():
    return ''.join(random.choice('0123456789ABCDEF') for _ in range(32))

def handle_error(response, context=""):
    if not response.ok:
        try:
            error_msg = response.json()
        except Exception:
            error_msg = response.text
        raise Exception(f"[{context} ERROR] Status {response.status_code}: {error_msg}")

def device_exists(device_id):
    # Prüfe in der Application Registry
    url = f"{TTI_API_BASE}/applications/{APPLICATION_ID}/devices/{device_id}"
    response = requests.get(url, headers=headers)

    if response.status_code == 200:
        return True
    elif response.status_code == 404:
        return False
    else:
        # Für Debugging und Fehleranalyse
        print(f"Unexpected response while checking device '{device_id}': {response.status_code}")
        response.raise_for_status()

def delete_device(device_id):
    # Lösche zuerst aus JS, NS, AS (Reihenfolge JS → NS → AS empfohlen)
    for service in ["js", "ns", "as"]:
        url = f"{TTI_API_BASE}/{service}/applications/{APPLICATION_ID}/devices/{device_id}"
        resp = requests.delete(url, headers=headers)
        if resp.status_code == 404:
            print(f"{service.upper()}: Device '{device_id}' not found.")
        else:
            print(f"{service.upper()}: Delete status {resp.status_code}")

    # Dann aus der Application Registry löschen
    url = f"{TTI_API_BASE}/applications/{APPLICATION_ID}/devices/{device_id}"
    resp = requests.delete(url, headers=headers)
    if resp.status_code == 404:
        print(f"APPLICATION: Device '{device_id}' not found.")
    else:
        print(f"APPLICATION: Delete status {resp.status_code}")


def create_in_ns(device_id, dev_eui, join_eui):
    url = f"{TTI_API_BASE}/ns/applications/{APPLICATION_ID}/devices/{device_id}"
    data = {
        "end_device": {
            "ids": {
                "device_id": device_id,
                "dev_eui": dev_eui,
                "join_eui": join_eui
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
    response = requests.put(url, headers=headers, json=data)
    handle_error(response, "Create in ns")


def create_in_application(device_id, dev_eui, join_eui):
    url = f"{TTI_API_BASE}/applications/{APPLICATION_ID}/devices"
    data = {
        "end_device": {
            "ids": {
                "device_id": device_id,
                "dev_eui": dev_eui,
                "join_eui": join_eui,
                "application_ids": {
                    "application_id": APPLICATION_ID
                }
            },
            "name": device_id,
            "description": "Auto-created by script",
            "supports_join": True,
            "lorawan_version": "1.0.3",
            "lorawan_phy_version": "1.0.3-a",
            "network_server_address": "eu1.cloud.thethings.network",
            "application_server_address": "eu1.cloud.thethings.network",
            "join_server_address": "eu1.cloud.thethings.network"
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
    response = requests.post(url, headers=headers, json=data)
    handle_error(response, "Application Registry")



def create_in_as(device_id):
    url = f"{TTI_API_BASE}/as/applications/{APPLICATION_ID}/devices/{device_id}"
    data = {
        "end_device": {
            "ids": {"device_id": device_id},
        },
        "field_mask": {"paths": ["ids"]}
    }
    response = requests.put(url, headers=headers, json=data)
    handle_error(response, "AS Registry")

def create_in_js(device_id, dev_eui, join_eui, app_key):
    url = f"{TTI_API_BASE}/js/applications/{APPLICATION_ID}/devices/{device_id}"
    data = {
        "end_device": {
            "ids": {"device_id": device_id, "dev_eui": dev_eui, "join_eui": join_eui},
            "root_keys": {"app_key": {"key": app_key}}
        },
        "field_mask": {"paths": ["ids", "root_keys"]}
    }
    response = requests.put(url, headers=headers, json=data)
    handle_error(response, "Join Server Registry")

# --- Hauptprogramm ---
def main():
    base_dir = Path(__file__).parent.parent
    configs_dir = base_dir / "ttn_configs"
    configs_dir.mkdir(parents=True, exist_ok=True)

    device_ids = sensemate_ids + sensegate_ids  # Beispielhafte Device-IDs

    for device_id in device_ids:
        if device_exists(device_id):
            print(f"Device '{device_id}' exists, deleting it first...")
            delete_device(device_id)

        print(f"Creating device '{device_id}'...")
        dev_eui = generate_random_eui()
        join_eui = "0000000000000000"  # z.B. für TTI Standard
        app_key = generate_random_key()

        create_in_application(device_id,dev_eui,join_eui)
        create_in_js(device_id,dev_eui,join_eui,app_key)
        create_in_ns(device_id, dev_eui, join_eui)
        create_in_as(device_id)
        

        info = {
            "DevEUI": dev_eui,
            "JoinEUI": join_eui,
            "AppKey": app_key
        }

        with open(configs_dir / f"{device_id}_config.mk", "w") as f:
            f.write(f'CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\\"{join_eui}\\"\n')
            f.write(f'CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\\"{dev_eui}\\"\n')
            f.write(f'CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\\"{app_key}\\"\n')


if __name__ == "__main__":
    main()
