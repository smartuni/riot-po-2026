# SenseGate Firmware Documentation

Welcome. This folder documents the **SenseGate** firmware -- the embedded software that runs on physical flood gate sensor nodes in the BAI5 IoT floodgate monitoring system.

## File Index

| File | Description |
|------|-------------|
| `01-overview.md` | What SenseGate is, how it fits into the overall system, and its hardware. |
| `02-setup.md` | Prerequisites, building, flashing, identity provisioning, and troubleshooting. |
| `03-architecture.md` | Code structure, component tree, RIOT OS integration, and custom modules. |
| `04-sensor-logic.md` | Sensor detection, gate state machine, interrupt handling, secure messaging, and LoRaWAN communication. |
| `05-health-monitoring.md` | Health monitoring that wraps both battery voltage monitoring and shock detection. |

## Suggested Reading Order

If you are new to this project, read in this order:

1. **01-overview.md** -- Understand the big picture: what SenseGate is and where it sits in the system.
2. **02-setup.md** -- Learn how to get the firmware onto a real device.
3. **03-architecture.md** -- See how the code is organized and which modules do what.
4. **04-sensor-logic.md** -- Dive into how the sensor detects gate position and sends data to the cloud.
5. **05-sensor-logic.md** -- Learn how the health monitor works, including battery and shock detection, which communicate each other via IPC mechanisms.

## Glossary

| Term | Meaning |
|------|---------|
| **RIOT OS** | An open-source real-time operating system for IoT devices. Like Linux, but for tiny microcontrollers. |
| **nRF52840** | A low-power Bluetooth/802.15.4 microcontroller from Nordic Semiconductor. The "brain" of SenseGate. |
| **MCU** | Microcontroller Unit -- a small computer on a single chip. |
| **LoRaWAN** | Long Range Wide Area Network -- a radio protocol for sending small amounts of data over several kilometers with very low power. |
| **TTN** | The Things Network -- a free, community-run LoRaWAN network that forwards device messages to the internet. |
| **MQTT** | A lightweight messaging protocol. The backend receives LoRaWAN data via MQTT. |
| **CBOR** | Concise Binary Object Representation -- a compact binary data format (like JSON, but smaller and faster to process). |
| **COSE** | CBOR Object Signing and Encryption -- a standard for digitally signing and encrypting CBOR data. |
| **HLC** | Hybrid Logical Clock -- a timestamp mechanism that combines wall-clock time with a logical counter, so events can be ordered even when clocks are not perfectly synchronized. |
| **Inductive sensor** | A sensor that detects metal objects nearby without touching them. SenseGate uses it to detect whether the flood gate is open or closed. |
| **GPIO** | General Purpose Input/Output -- a pin on the MCU that can read or write digital signals. |
| **ADC** | Analog-to-Digital Converter -- converts an analog voltage (e.g. from a sensor) to a digital number the MCU can process. |
| **OTAA** | Over-The-Air Activation -- the secure method LoRaWAN devices use to join a network. |
| **DevEUI / JoinEUI / AppKey** | LoRaWAN credentials: a unique device ID, a network identifier, and an application encryption key. |
