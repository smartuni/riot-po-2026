# `identity_store`

## Overview

The `identity_store` module handles the storage of BLE as well as LoraWAN keys for a node. This keeps the firmware image free of any secrets and unique IDs.

For this it utilizes the external flash storage on the board and serves as an abstraction on the filesystem and directory structure it manages.

This helps to facilitate a key-gossiping mechanism in which the nodes can receive and permanently store other node's public keys which have been signed by the root private key.

It provides functions for:
* Setting up a node
* Getting LoraWAN keys
* Getting the root public key
* Getting your own key ID
* Getting your own private identity
* Getting your own public identity
* Getting your own signed public identity, which has been signed by the root private key
* Adding other node's signed public identities, which have been signed by the root private key
* Getting other node's public identities

## Identities

The identity store operates on what we call `identities`: A pair of either public or private key and a corresponding key ID, which will be used for BLE communication.

A `signed (public) identity` is a pair of a CBOR encoded `intentity` and the signature obtained from signing that data with the root private key.

## Storage layout

Itentity files and LoraWAN keys are stored in the following structure on the external flash:

```
/config
└─/loramac
  ├─ joineui
  ├─ deveui
  └─ nwkkey
/identities
├─/self
│ ├─ root.pubid
│ ├─ self.pubid
│ └─ self.prvid
├─/valid
│ ├─ sensemate-001
│ ├─ sensemate-002
│ ├─ ...
│ ├─ sensegate-001
│ ├─ sensegate-002
│ └─ ...
└─/revoked
```