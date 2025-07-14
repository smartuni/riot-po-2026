# Description

This module uses the RIOT LoRaWan module and adapts it to fit the requirements of the nodes of the SenseMate project.

The module is used to send messages to the server using an uplink.

# Data structure

CBOR Example
```
[
    1,    # 1 is an example value for the message type
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

