# RIOT im Internet of Things, Sommersemester 2026

**Document Index**
- [RIOT im Internet of Things, Sommersemester 2026](#riot-im-internet-of-things-sommersemester-2026)
  - [Project Introduction](#project-introduction)
  - [The Current Situation](#the-current-situation)
    - [Propesed Solution](#propesed-solution)
  - [1. Getting started](#1-getting-started)
    - [Nodes](#nodes)
      - [Build prerequisities](#build-prerequisities)
      - [Building firmware](#building-firmware)
      - [Flashing firmware](#flashing-firmware)
      - [Provisioning devices](#provisioning-devices)
        - [Setup](#setup)
          - [Provisioning](#provisioning)
    - [Hardware](#hardware)
    - [Dashboard - Server (Frontend \& Backend)](#dashboard---server-frontend--backend)
      - [Prerequisites \& Installation](#prerequisites--installation)
      - [MQTT connection setup](#mqtt-connection-setup)
      - [Starting the Dashboard](#starting-the-dashboard)
  - [2. Troubleshooting](#2-troubleshooting)
    - [Flashing / Firmware](#flashing--firmware)
    - [Backend / Frontend](#backend--frontend)
    - [General](#general)
  - [3. Documentation Index](#3-documentation-index)
    - [External References](#external-references)
  - [4. Notes for future reference:](#4-notes-for-future-reference)
    - [Incomplete features:](#incomplete-features)

## Project Introduction

Welcome to the primary repository of the RIOT im Internet of Things project for the Summer 2026 semester at HAW Hamburg. The project is concerned with designing, implementing and presenting technologies to improve the city of Hamburg's methods of managing its harbor's floodgates.

## The Current Situation

Field workers use walkie-talkies to report floodgate status. Central office logs everything on paper. This is slow, error-prone, and creates no digital trail — a critical risk during floods.

### Propesed Solution

An IoT system with three tiers:

| Tier | Device | Role |
|------|--------|------|
| **SenseGate** | Fixed sensor node (nRF52840 + LoRaWAN) | Mounted on flood gates, detects open/closed state, reports via LoRaWAN |
| **SenseMate** | Handheld device (nRF52840 + BLE + OLED) | Field workers see gate states, record observations, receive alerts |
| **Server** | Spring Boot + React Dashboard | Central data hub: REST API, real-time WebSocket, MQTT from TTN |

**Data flow:** Gate movement → SenseGate sensor → CBOR+COSE sign → LoRaWAN uplink → TTN → MQTT → Backend → WebSocket → Frontend dashboard.

---

## 1. Getting started

### Nodes

#### Build prerequisities

| Tool | Purpose | 
|------|---------|
| **RIOT OS** | Operating system for the firmware |
| **Make** | Automates the Build Process |
| **Docker** | `riot/riotbuild:2025.07` — prebuilt toolchain |
| **Python 3 + uv** | Runs the identity-manager provisioning script |


**RIOT Submodule**

First, make sure that the riot submodule is downloaded:
```bash
git submodule init
git submodule update
```

**Install make (Linux - bash)**

```bash
sudo apt update
sudo apt install make build-essential
```

**Install Docker Engine**

- [All Installation Guides](https://docs.docker.com/engine/install/)

**Alternative - Docker Desktop**

- [Install on Mac](https://docs.docker.com/desktop/setup/install/mac-install/)

- [Install on Windows](https://docs.docker.com/desktop/setup/install/windows-install/)

- [Install on Linux](https://docs.docker.com/desktop/setup/install/linux/)

**Install UV**

```
# On macOS and Linux.
curl -LsSf https://astral.sh/uv/install.sh | sh
```
```
# On Windows.
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
```

[More installation guides and information](https://github.com/astral-sh/uv)


To use a python venv, run the following commands in the root directory of the project:
```bash
uv venv --seed
source .venv/bin/activate
```
>Note: some Linux distributions no longer allow installing python packages via pip directly. It is recommended to use a venv instead for easier package management.

#### Building firmware

Navigate to the respective folders for senseMate or senseGate in `nodes/firmware/applications/sense{Mate|Gate}`

> Note: To build the firmware for the previous (v1) hardware revision, instead run:
> ```bash
> BOARD=adafruit-feather-nrf52840-sense make all -j
> ```


Set environment variable to use the docker build stack:
```bash
export BUILD_IN_DOCKER=1
```

Build the firmware using 
```bash
make all -j
```
>Note: when building for the first time, the entire build toolchain will be set up in the docker container automatically. This will take some time and download several GB of data. Make sure to have at least 20 GB of storage available for this.

#### Flashing firmware

To flash, make sure to install the following python dependencies using `pip install` (ideally in the uv venv set up ealier):
- pyserial
- psutil

Make sure that the user account doing the flashing has the necessary permission, e.g. being in the `dialout` group on linux or similar for other operating systems.

Flash the previously built firmware using
```bash
make flash
```

If no identity is provisioned on the external flash (like after inital flashing of a new board), the screen of a senseMate will stay blank after successfully flashing and no (or not much) log output will appear.

#### Provisioning devices

Provisioning of devices is done via the identity-manager module found at `/nodes/firmware/identity-manager/`

> Note: uv must be installed to run the identity manager.

##### Setup

To use the credential manager, it needs to be set up.
First, create the config for the credential manager at `nodes/firmware/identity-manager/config.yaml`. See the example config (`config.examle.yaml`) for reference.

Now, log in to the [TTN Console](https://eu1.cloud.thethings.network) and navigate to Applications → your application → API keys, create a new API key, and put it into the config. Update your application ID if necessary. 

Next, either import an existing root identity into `nodes/firmware/identity-manager/identities/root`, or create a new one using the identity manager as follows:

```bash
./identity-manager.py root create
```
>Note: the same root identity must be used for all nodes for them to be able to communicate with each other successfully.

Next, individual node identities need to be configured. These can either be imported by putting their respective ID info files into `nodes/firmware/identity-manager/identities/node/` or by creating them using the identity manager as follows:

```bash
./identity-manager.py node create [senseMate|senseGate] [-i 1]
```
This command creates a new identity for either a senseMate or a senseGate with the ID 1.
The ID parameter (`-i [id]`) is optional and can be omitted, in that case the next free ID gets chosen

###### Provisioning

When provisioning a node, it is best practice to wipe the external storage, so a subsequent flash is going to succeed without issues.
This is done by running:
```bash
./identity-manager.py node wipe
```

Now the node can be provisioned with the previously generated identity:
```bash
./identity-manager.py node provision [senseMate|senseGate] ID
```

>Warning: The wipe and provision commands try to communicate with the node via `/dev/ttyACM0`. Make sure to only have one node connected at a time to avoid confusion. If the communication fails, check if another device is already registered.

>Note: Once a node is provisioned, flashing a new firmware on it does not affect the identity as it is stored on the flash. This provisioning step only needs to be done once (unless the identity gets deleted from the flash somehow).


More info can be found in the [README of the identity-manager module](nodes/firmware/identity-manager/README.md).

---

### Hardware

**SenseGate** — 2-layer carrier PCB for XIAO nRF52840 Sense, with 4× reed switches, inductive sensor, power management (KiCad 10.0, gerbers ready).

**SenseMate** — 4-layer PCB integrating XIAO nRF52840 Sense Plus, LoRa module (RFM95W-868S2), SSD1306 OLED, buzzer, vibration motor (KiCad 10.0, gerbers ready).

No physical modifications needed — both PCBs are designed for the XIAO module with standard pin headers. See:

- [SenseGate Hardware Documentation](./documentation/Hardware/SenseGate/README.md) — full component list, pin mapping, schematic references
- [SenseMate Hardware Documentation](./documentation/Hardware/SenseMate/README.md) — full component list, pin mapping, schematic references
- [Hardware Design Files](./nodes/hardware/) — KiCad schematics, PCB layouts, gerber files

**Install Kicad**

- [Install for Linux](https://www.kicad.org/download/linux/)

- [Install for Mac](https://www.kicad.org/download/macos/)

- [Install for Windows](https://www.kicad.org/download/windows/)

**Arch Linux**
```
#Install on Arch Linux with Libraries
sudo pacman -Syu kicad
# if you want to install the official libraries (recommended):
sudo pacman -Syu --asdeps kicad-library kicad-library-3d
```


> Note: KiCad has a Lightversion without 3D components. We recommend the Full version that includes 3D components. 

---

### Dashboard - Server (Frontend & Backend)

#### Prerequisites & Installation

| Tool | Version | Purpose | Installation |
|------|---------|---------|-------------|
| **Java** | 17+ | Runs the Spring Boot backend | [OpenJDK 17](https://adoptium.net/) |
| **Node.js** | 20+ | Runs the React frontend | [nodejs.org](https://nodejs.org/) |
| **Docker** (optional) | latest | Containerized full-stack startup | [docker.com](https://www.docker.com/) |
| **Maven** (optional) | 3.9+ | Java build tool (wrapper included) | Bundled as `./mvnw` |

> **Windows / Mac:** Install Java 17 and Node.js 20+. Maven wrapper (`mvnw`) works cross-platform.
> **Linux:** Same — install via package manager (`apt install openjdk-17-jdk nodejs npm`).


#### MQTT connection setup

to obtain MQTT credentials for the connection to the things network, log into the [TTN Console](https://eu1.cloud.thethings.network)

Navigate to Applications → your application → other integrations → MQTT
Generate a new API key here, and insert the following into `server/backend/src/main/resources/application.yml`:

```
mqtt:
  broker: ssl://eu1.cloud.thethings.network:8883
  clientId: testing ← this does not matter
  username: hawriotfloodgates@ttn ← you find this on the mqtt page
  applicationId: hawriotfloodgates ← your application ID
  password: NNSXS.XXXXXXXXXX.XXXXXXX ← the API key goes here
  subscribeTopic: v3/hawriotfloodgates@ttn/devices/+/up  ← put the username in here
```


>Hint: when starting the backend container, look for the following lines in the output:
>
>```
>MQTT Publisher verbunden
>MQTT-Client gestartet, warte auf Nachrichten...
>```
>If these don't appear, double check your configuration and make sure that the MQTT broker is reachable via the network. As of summer 2026, port 8883 is blocked in the HAW network, so use a VPN or mobile hotspot if necessary. 

#### Starting the Dashboard

Start the containers with `docker compose up`

>Note: when testing out new changes to the code, make sure to delete the built container images using `docker image rm <image>` before running them again to get the latest state.

After full startup, the dashboard is reachable via the browser at http://localhost:3000

To stop the containers, use `docker compose down`.
To stop and delete the volumes with the database state, use `docker compose down -v`.

---

## 2. Troubleshooting

### Flashing / Firmware

| Problem | Cause | Solution |
|---------|-------|----------|
| `make flash` fails with linker errors | Signature keys missing | Generate keys in `key-distro/include/secrets/` |
| LoRaWAN join fails | Wrong TTN credentials | Check DevEUI/JoinEUI/AppKey in identity-manager and TTN Console |
| Device doesn't appear on dashboard | Not provisioned | Run `identity-manager node provision` |
| Blue LED never turns on | Main loop crashed | Re-flash, check serial output (115200 baud) |
| `periph_pwm` build error | Board doesn't support PWM (SenseMate) | Use `seeedstudio-xiao-nrf52840-sense` board |
| Bootloader not found | Wrong board selected | Default: `BOARD=seeedstudio-xiao-nrf52840-sense` |

### Backend / Frontend

| Problem | Cause | Solution |
|---------|-------|----------|
| Backend won't start (port conflict) | Port 8080 already in use | Kill process or change port in `application.yml` |
| Frontend can't reach backend | Vite proxy misconfigured | Check `vite.config.js` — `/api` should proxy to `localhost:8080` |
| MQTT connection refused | Wrong TTN broker or API key | Verify `.env` credentials, check TTN Console |
| WebSocket not updating | Wrong WebSocket endpoint | Backend uses STOMP at `/ws` — see [Frontend real-time docs](./documentation/Frontend/04-state-management.md) |
| Database migration fails | Wrong Spring profile | Use `e2e` (H2) for testing, `dev` (PostgreSQL) with running DB |

### General

- **Git submodules not cloned?** Run `./submodules_linux.sh`
- **Docker not starting?** Ensure Docker daemon is running (`systemctl start docker`)
- **Windows line endings?** Use `git config core.autocrlf input` before cloning

---

## 3. Documentation Index

| Document | What You'll Find |
|----------|------------------|
| [SenseGate Firmware](./documentation/Firmware/SenseGate/README.md) | Sensor logic, gate observer state machine, LoRaWAN uplink, COSE/CBOR data flow |
| [SenseMate Firmware](./documentation/Firmware/SenseMate/README.md) | LVGL OLED UI, BLE communication, sound/vibration subsystems, HLC timestamps |
| [SenseGate Hardware](./documentation/Hardware/SenseGate/README.md) | PCB components, pin mapping table with firmware references, RIOT board hierarchy |
| [SenseMate Hardware](./documentation/Hardware/SenseMate/README.md) | PCB v2 components, 28-pin XIAO mapping, LoRa/display/buzzer pinout |
| [Backend](./documentation/Backend/README.md) | REST API endpoints (Auth, Gates, Nodes, Notifications), JPA entities, Flyway migrations, MQTT/WebSocket setup |
| [Frontend](./documentation/Frontend/README.md) | React 19 + Redux Toolkit architecture, feature-based structure, STOMP WebSocket middleware, auth flow |

### External References

| Resource | Purpose |
|----------|---------|
| [Identity Manager](./nodes/firmware/identity-manager/README.md) | Python tool for provisioning node identities and LoRaWAN credentials |
| [Key Distribution](./nodes/firmware/custom-modules/key-distro/README.md) | Cryptographic key generation for COSE signing |
| [CI Pipeline](./.gitlab-ci.yml) | GitLab CI — firmware tests, compilations, web tests, Docker builds |
| [Architecture Diagrams](./documentation/) | System context, technical views, backend architecture (PNG) |


## 4. Notes for future reference:
Notes from the summer 2026 project group to future contributors.

The working state presented at the summer 2026 presentation can be found at the git tag `presentation-summer-2026`

### Incomplete features:

- SenseMate: The Vibration works and is tested but not activated in the firmware
- DockingStation: Data connection between computer and SenseMate isn't working. The Pins for Communication with Pogopins needs to be tested and activated in the firmware of the SenseMate. 
- Dockingstation: USB Connection is a quickfix - Connection to the pogo pins should be improved. Maybe a dedicated PCB.
