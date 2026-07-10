# `identity-manager.py`

Script for managing identities as well as related functionality:

- Creation of a root identity for signing other identities and for use as a trust-anchor.
- Creation of node identities for setting up nodes (like senseMates and senseGates) providing:
    - Private key for the node.
    - Signed public identity for the node, so it can be gossiped successfully to other nodes.
    - The Things Network (LoraWAN) configuration.
    - Provisioning payload holding all this information for node setup.
- Provisioning of nodes.
- Wiping of nodes.

See the help page of the command for more information on the commands and how to use them. Also see the [Quickstart](#quickstart).  
Note that a [configuration file](#configuration) needs to be provided under `config.yaml` next to the script.  
The generated identity information will be stored in [identity information files](#_id_info-files) in the [`identities` directory](#identities-directory) next to the script.

## Quickstart

To set up a new node, the following steps should be taken:

1. If one doesn't already exist, create a new root identity using:
   ```shell
   ./identity-manager.py root create
   ```
2. Generate a new identity for the node using the following command (e.g. provisioning a new senseMate):
   ```shell
   ./identity-manager.py node create senseMate
   ```
   Ensure a [configuration](#configuration) is present under the `config.yaml` for this.
3. If the node is already provisioned, wipe it using:
   ```shell
   ./identity-manager.py node wipe
   ```
4. Finally provision the node using the following command (e.g. provisioning a new senseMate):
   ```shell
   ./identity-manager.py node provision senseMate ID
   ```
   The ID should be the ID generated in step 1.

## Configuration

A configuration file should be present next to the script under `config.yaml`. It's easiest to just copy the provided `config.example.yaml` and change the values as necessary.

The configuration file has the following structure:

- `ttn`:
    - `instance`: The Things Network instance to use. Should most likely be left as is.  
      Defaults to: `eu1.cloud.thethings.network`.
    - `applicationID`: The Things Network application ID to use. Can be found in The Things Network web interface.
    - `authToken`: The Things Network auth token to use. Can be created in The Things Network web interface (https://eu1.cloud.thethings.network/console/applications/YOUR-APPLICATION-ID/api-keys).

## `identities` directory

The directory has the following structure:

```
/identities
├─/root
│ └─ root_000.root_id_info.json
└─/nodes
  ├─ senseMate_001.node_id_info.json
  ├─ senseMate_002.node_id_info.json
  ├─ ...
  ├─ senseGate_001.node_id_info.json
  ├─ senseGate_002.node_id_info.json
  └─ ...
```

The identities are split-up in two directories depending on whether or not they are root or node identities. This is simply done for convenience reasons.

## `*.*_id_info.json` files

The identity information files adhere to a structure outlined in the following.

### `*.root_id_info.json` files

JSON files for root identity information with the following structure:

```json
{
    "kid": "00000300",
    "privateKey": "-----BEGIN PRIVATE KEY-----\n...",
    "publicKey": "-----BEGIN PUBLIC KEY-----\n..."
}
```

- `kid`: Root (Key)ID as described in TODO represented as a hex string.
- `privateKey`: Root private key in PKCS #8 and PEM format.
- `publicKey`: Root public key in PEM format for reference only.

### `*.node_id_info.json` files

JSON files for node identity information with the following structure:

```json
{
    "kid": "0000010a",
    "privateKey": "-----BEGIN PRIVATE KEY-----\n...",
    "publicKey": "-----BEGIN PUBLIC KEY-----\n...",
    "signedPublicKey": {
        "publicIdentity": "8244...",
        "signature": "d284..."
    },
    "ttn": {
        "joinEUI": "00...",
        "devEUI": "45...",
        "appKey": "E9..."
    },
    "rootKey": "-----BEGIN PUBLIC KEY-----\n...",
    "provisioningPayload": "..."
}
```

- `kid`: (Key)ID of the node as described in TODO represented as a hex string.
- `privateKey`: Node private key in PCKS #8 and PEM format.
- `publicKey`: Node public key in PEM format.
- `signedPublicIdentity`: Node signed public identity, signed by the root key (using COSE Sign1).
    - `publicIdentity`: The node public identity as a CBOR message represented as a hex string.
    - `signature`: The signature of the node public identity. More accurately this is a COSE Sign1 message represented as a hex string, where the `publicIdentity` is a detached payload.
- `rootKey`: Root key in PEM format used for signing the node public identity.
- `ttn`: The Things Network (LoraWAN) configuration.
    - `joinEUI`: JoinEUI
    - `devEUI`: DevEUI
    - `appKey`: AppKey
- `provisioningPayload`: base64 encoded payload holding all the information for provisioning the node.
