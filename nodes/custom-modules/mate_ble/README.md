# Description

This module uses the RIOT BLE module and adapts it to fit the requirements of the nodes of the SenseMate project.

Instead of sending messages to one specific device, the BLE module enables the node to act as a beacon to transmit its information to all nodes within a certain radius.

# Data structure

The BLE module requires CBOR to encode the table type that is being transmitted, the timestamp, the type of device that is sending a message (either 0 for SenseGate or 1 for SenseMate) and the deviceID in order to process the received information.

CBOR Example
```
[
    1,    # 1 is an example value for the message type
    247,  # Timestamp
    1,    # 1 is an example of a type of device
    5,    # 5 is an example deviceID
    [     # The list with the "Soll Status" entries
        [ # This is a "Soll Status" entry
            187, # GateID
            0,   # Soll Status
        ],
        [ # 2nd "Soll Status" entry
            69,  # GateID
            1,   # Soll Status
        ]
        # ... More entries
    ]
]
```

