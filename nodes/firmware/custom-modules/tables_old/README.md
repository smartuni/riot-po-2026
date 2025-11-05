# Description

The tables module provides CRUD functionality for the tables. 

Additionally, the module provide functionality to encode tables to CBOR and decode CBOR packages to table structs.
Consider to use set, get and merge functions for modyfying the table because these functions are thread-safe instead of getting direct pointer to the tables.
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

## Is state table (Node-to-Node, Node-to-Server)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| State | BYTE | Current state of a gate: closed or opened |
| Gate Time | Int | Last update timestamp |

CBOR Example
```
[
    2, # 2 is an example value for the message type
    247,  # Timestamp
    1,    # 1 is an example for which type of device sent the message
    5,    # 5 is an example for a deviceID
    [  # The list with the "is state" entries
        [ # This is a "is state" entry
            187, # GateID
            0,   # Is state
            247  # Timestamp
        ],
        [ # 2nd "is state" entry
            69,  # GateID
            1,   # Is state
            333  # Timestamp
        ]
        # ... More entries
    ]
]
```

## Seen State / Worker Input (Node-to-Node, Node-to-Server)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Gate Time | Int | timestamp from a gate |
| Status | BYTE | State of the gate: closed or opened |
| SenseMate ID | Int | ID of SenseMate of the worker who confirmed the state |

## Jobs Table (Server-to-Node(SenseMate))

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Progress | BYTE | Progress of task, either done or in progress |
| Priority | BYTE | - |

## Timestamp table

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Timestamp | uint32 | last timestamp receiving from a cbor packet of a gate |
| Distance / Signal strength | Int | signal strength of SenseMate that was near the gate |
