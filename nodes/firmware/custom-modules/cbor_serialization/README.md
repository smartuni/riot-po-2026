# `cbor_serialization`

## Overview

The `cbor_serialization` module handles [CBOR](https://datatracker.ietf.org/doc/html/rfc8949) encoding and decoding of messages received via BLE (via [`mate_ble`](../mate_ble/)) and LoraWAN (via [`mate_lorawan`](../mate_lorawan/)). The `identity_store` module also heavily utilizes it for CBOR encoding and decoding.

## Message Structure

On the top level each message is a flat array of data with the version and message type information shared across all types of messages.

```
[
 01, # version
 01, # message type
]
```

Depending on the type of message, the message then contains additional information.  
The following message types are available:

- **single report** (`MESSAGE_TYPE_SINGLE_REPORT`, `0x01`)
- **ID request** (`MESSAGE_TYPE_ID_REQUEST`, `0x02`)
- **ID response** (`MESSAGE_TYPE_ID_RESPONSE`, `0x03`)

### single report (`MESSAGE_TYPE_SINGLE_REPORT`, `0x01`)

```
[
  01, # version
  01, # message type

  ### record header ###
  01, # record type
  01, 02, 03, 04, # writer ID as bytestr
  00, 00, 00, 00, 00, 00, 00, 01, # sequence number
  ## HLC ##
  00, 00, 12, 34, # physical timestamp
  00, 00, 00, 0A, # logical timestamp

  ### gate report ###
  01, # gate state

  ### gate observation ###
  01, 02, 03, 05, # gate ID as bytestr
  01, # gate state

  ### gate command ###
  01, 02, 03, 05, # gate ID as bytestr
  01, # gate state

  ### gate job ###
  01, 02, 03, 05, # gate ID as bytestr
  01, 02, 03, 04 # mate ID as bytestr
  01, # gate state

  ### signature ###
  bstr()
]
```

### **ID request** (`MESSAGE_TYPE_ID_REQUEST`, `0x02`)

```
[
  01, # version
  02, # message type
  bstr(), # sender public identity
  bstr() # sender public identity signature
]
```

### **ID response** (`MESSAGE_TYPE_ID_RESPONSE`, `0x03`)

```
[
  01, # version
  03, # message type
  bstr(), # sender public identity
  bstr() # sender public identity signature
]
```
