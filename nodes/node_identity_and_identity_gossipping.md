# Node Identity and Identity Gossipping

## Node Identity

All the information unique to a node is stored only on its external flash. This means a firmware (for the right device type, e.g. `senseGate` or `senseMate`) can simply be flashed to multiple nodes without issue.  
Specifically the following information are stored on the flash of each node:

- its own identity, including:
    - its private key
    - its signed public key
    - its ID
- the trusted root identity
- the TTN (The Things Network) credentials and information unique to the node

A detailed description of what is stored where and how it's managed in the firmware can be found in the [README of the `identity_store` module](firmware/custom-modules/identity_store/README.md).  
The [README of the `identity-manager` script](firmware/identity-manager/README.md) includes further information as well, with instructions on how to generate and apply these various identities and credentials.

## Identity Gossipping

Each node then also stores a list of known trusted public keys of other nodes on its external flash.
- Nodes learn other nodes identities by gossipping identities via BLE.
    - Currently, each node simply sends its own signed identity - this is an area for improvement.
- When receiving a new signed identity, a node verifies that it is signed by the trusted root identity, only then it is added to the trusted public identities.
- A description of the package payloads can be found in the [README of the `cbor_serialization` module](firmware/custom-modules/cbor_serialization/README.md).

## Provisioning a New Node

Internally the `identity-manager.py` script sends all the relevant information encoded in base64 to the node over serial, which then decodes and saves it.
