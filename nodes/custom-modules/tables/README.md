## Target state table 

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| target state | BYTE | Information whether a gate should be closed or opened |

CBOR Example
```
[
    1,    # 1 is an example value for the message type
    247,  # Timestamp
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

## Ist Status Tabelle (BLE)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Status | BYTE | Information whether a gate is closed or opened |
| Gate Time | Int | Time when the status was updated |

CBOR Example
```
[
    2, # 2 is an example value for the message type
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

## Gesehener Status / Mitarbeiter Input (BLE)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Gate Time | Int | Time when the information was confirmed |
| Status | BYTE | Information whether a gate is closed or opened |
| SenseMate ID | Int | ID of SenseMate of the worker who confirmed the status |

## Aufgaben für Mitarbeiter (DL, Server to one SenseMate)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |

## Location Tracking (UL)

On SenseMate device

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Gate Time | Int | Time when the SenseMate was near the gate |
| Distance / Signal strength | Int | signal strength of SenseMate that was near the gate |

# Offene Fragen
- Frequenz beachten, wie oft Nachrichten geschickt werden
- State Tables verwalten
- Fertige CBOR Nachricht wird weitergegeben, sodass signierte Cozy Nachricht daraus entsteht
