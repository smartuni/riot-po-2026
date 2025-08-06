# BLE Module

## Overview

The BLE module builds on RIOT’s NimBLE stack to provide beacon‐style, peer‐to‐peer exchange of state tables between SenseMate and GateMate nodes. Instead of point‐to‐point messaging, each node broadcasts its current state and listens for updates from its neighbors.

## Key Features

- **Beacon Broadcasting**  
  Periodically advertises state tables to all nearby nodes.

- **Concurrent TX/RX**  
  Runs two dedicated threads:
  1. **Transmitter** – encodes and broadcasts your node’s state tables.  
  2. **Receiver** – listens for incoming tables, verifies, and forwards them to the `tables` module.

- **Expected Payloads**  
  - **CBOR** for compact, architecture independent encoding 
  - **COSE** signatures (via `cose-service`) to guarantee authenticity & integrity

## Dependencies

- **RIOT OS** (NimBLE and other RIOT features)  
- **cose-service** (for COSE signing and verification)  
- **table-module** (to prepare outgoing and process incoming packets)

## Paket Layout

The BLE module requires CBOR to encode the table type, the timestamp and the transmitters device type and device-id in order to process the received information.

The packets need to be signed using the "cose-service" module.

Example of a CBOR packet
```json
[
  1,              // Message type
  247,            // Timestamp
  1,              // Device type
  5,              // Device ID
  [               // State entries array
    [187, 0],     // GateID=187, Status=0
    [1337, 1]     // GateID=1337, Status=1
    // … more entries
  ]
]
```