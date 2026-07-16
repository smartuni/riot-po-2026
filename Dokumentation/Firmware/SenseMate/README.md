# SenseMate Firmware Documentation

Documentation for the SenseMate firmware — the handheld companion device in the BAI5 IoT floodgate monitoring system.

## File Index

| File | Description |
|------|-------------|
| `01-overview.md` | What SenseMate is, its role in the system, and hardware overview |
| `02-setup.md` | Prerequisites, build commands, key generation, identity provisioning, troubleshooting |
| `03-architecture.md` | Application structure, RIOT OS integration, custom modules, Makefile explanation |
| `04-ui-subsystem.md` | LVGL-based OLED display, sound subsystem (buzzer), vibration motor, event handling |
| `05-communication.md` | BLE, LoRaWAN, CBOR serialization, COSE cryptography, message flow |

## Suggested Reading Order

1. **Start with** `01-overview.md` — get the "big picture" of what SenseMate does
2. **Then** `03-architecture.md` — understand the code structure and how modules fit together
3. **Then** `04-ui-subsystem.md` and `05-communication.md` — dive into the two main subsystems in any order
4. **Use** `02-setup.md` when you need to actually build and flash the device

## Glossary

| Term | Meaning |
|------|---------|
| **RIOT OS** | A real-time operating system for IoT devices. Think of it as a lightweight "mini-Linux" for microcontrollers. |
| **BLE** | Bluetooth Low Energy. A wireless technology for short-range communication between devices. SenseMate uses it to talk to SenseGates nearby. |
| **LoRaWAN** | Long Range Wide Area Network. A wireless protocol for long-range, low-power IoT communication. SenseMate uses it to send data to the cloud server. |
| **CBOR** | Concise Binary Object Representation. A binary data format (like JSON, but more compact). Used to encode messages between devices so they use minimal bytes. |
| **COSE** | CBOR Object Signing and Encryption. A standard for signing and encrypting CBOR messages. Ensures data coming from other devices is authentic. |
| **HLC** | Hybrid Logical Clock. A system for ordering events across devices that don't share a common clock. Combines physical time with a logical counter. |
| **LVGL** | Light and Versatile Graphics Library. An open-source graphics library for embedded devices. SenseMate uses it to draw the UI on its OLED screen. |
| **PWM** | Pulse Width Modulation. A way to control hardware by rapidly switching a signal on and off. Used here to drive the buzzer at different frequencies. |
| **GPIO** | General Purpose Input/Output. Physical pins on the microcontroller that can be controlled by software — set high (on) or low (off). |
| **MCU** | Microcontroller Unit. The "brain" of an embedded device. SenseMate uses the nRF52840. |
| **SSD1306** | A specific OLED display driver chip. SenseMate's 128x64 pixel display uses this. |
| **I2C** | Inter-Integrated Circuit. A communication protocol for connecting chips together. The OLED display connects to the MCU over I2C. |
| **MTD** | Memory Technology Device. An abstraction layer in RIOT OS for flash memory storage. |
| **FlashDB** | A lightweight key-value database for embedded flash storage. Used to persist credentials and table data. |
| **TTN** | The Things Network. A free, community-based LoRaWAN network infrastructure. |
| **Node ID** | A unique 4-byte identifier assigned to each device during provisioning. |
