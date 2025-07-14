# Description

The tables module includes several functions to work with the tables below. 

Additionally, there are several functions to encode these tables to CBOR as well as a function to decode CBOR back to this table format.

# Tables

## Target state table (Server-to-Node, Node-to-Node)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| target state | BYTE | Information whether a gate should be closed or opened |
| Timestamp | uint32 | Timestamp of time when message was sent by server |

CBOR Example
```
[
    1,    # 1 is an example value for the message type
    247,  # Timestamp
    0,    # 0 is an example for which type of device sent the message
    5,    # 5 is an example for a deviceID
    [     # The list with the "Soll Status" entries
        [ # This is a "Target Status" entry
            187, # GateID
            0,   # Soll Status
        ],
        [ # 2nd "Target Status" entry
            69,  # GateID
            1,   # Soll Status
        ]
        # ... More entries
    ]
]
```

## Ist Status Tabelle (Node-to-Node, Node-to-Server)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Status | BYTE | Information whether a gate is closed or opened |
| Gate Time | Int | Time when the status was updated |

CBOR Example
```
[
    2, # 2 is an example value for the message type
    247,  # Timestamp
    1,    # 1 is an example for which type of device sent the message
    5,    # 5 is an example for a deviceID
    [  # The list with the "Ist Status" entries
        [ # This is a "Ist Status" entry
            187, # GateID
            0,   # Ist Status
            247  # Timestamp
        ],
        [ # 2nd "Ist Status" entry
            69,  # GateID
            1,   # Ist Status
            333  # Timestamp
        ]
        # ... More entries
    ]
]
```

## Gesehener Status / Mitarbeiter Input (Node-to-Node, Node-to-Server)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Gate Time | Int | Time when the information was confirmed |
| Status | BYTE | Information whether a gate is closed or opened |
| SenseMate ID | Int | ID of SenseMate of the worker who confirmed the status |

CBOR Example
```
[
    2, # 2 is an example value for the message type
    247,  # Timestamp
    1,    # 1 is an example for which type of device sent the message
    5,    # 5 is an example for a deviceID
    [  # The list with the "Ist Status" entries
        [ # This is a "Ist Status" entry
            187, # GateID
            0,   # Ist Status
            247  # Timestamp
        ],
        [ # 2nd "Ist Status" entry
            69,  # GateID
            1,   # Ist Status
            333  # Timestamp
        ]
        # ... More entries
    ]
]
```

## Aufgaben für Mitarbeiter (Server-to-Node)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Progress | BYTE | Progress of task, either done or in progress |
| Priority | BYTE | - |

## Zeitstempel

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Timestamp | uint32 | Time when a gate last updated its data |
| Distance / Signal strength | Int | signal strength of SenseMate that was near the gate |
