
# Init git submodlues (RIOT) with
To init the submodules run submodules_linux.sh or submodules_windows.bat
You can also run these commands:
```
git submodule init
git submodule update
```

# Bluetooth Module

| Funktionsname | Parameters | Description |
| ----------- | ----------- | ----------- |
| send      | Message       | sends message as a beacon |
| receive   | Package Type | receive a pointer to a struct as well as meta data of the sender |
| init   | none        | initializes Bluetooth Module |

# LoRaWAN Module

| Funktionsname | Parameters | Description |
| ----------- | ----------- | ----------- |
| send | Message | sends message to server |
| receive | none | Text |
| init | none | initalizes LoRaWAN module |

# Datenstrukturen

## Soll Status Tabelle (BLE)

Timestamp is part of the package but not part of list

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Soll Status | BYTE | Information whether a gate should be closed or opened |
| Timestamp | Int | outside of package |

## Ist Status Tabelle (BLE)

| Name | Data Type | Description |
| ----------- | ----------- | ----------- |
| GateID | BYTE | - |
| Status | BYTE | Information whether a gate is closed or opened |
| Gate Time | Int | Time when the status was updated |

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
