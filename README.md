# RIOT im Internet of Things, Sommersemester 2026

Welcome to the primary repository of the RIOT im Internet of Things project for the Summer 2026 semester at HAW Hamburg. The project is concerned with designing, implementing and presenting technologies to improve the city of Hamburg's methods of managing its harbor's floodgates.

## The Current Situation

Currently, the methods for ensuring floodgates are closed in the event of a flood are inefficient. Field workers receive orders, send updates and otherwise communicate with the central office via walkie-talkies. The central office relies on pen-and-paper record-keeping to log events, such as gates being opened or closed.

## Our Proposed Solution

The solution proposed and developed by our team is as follows: 

- Floodgates will be fitted with sensors, called "senseGates," which autonomously report the state of the gate (open or closed) to a server located in the central office.
- Field workers will be issued devices, called "senseMates," which receive orders from the central office. senseMates will also record the state of nearby gates by communicating with the senseGates, enabling workers to confirm or refute the state reported by the senseGate.
- The workers at the office will be given a web app with a user interface, allowing them to track the status of gates in real time, as well as providing a digital record of the status of gates and allowing them to issue orders without the need for walkie-talkie communication.

Our proposed solution replaces the inefficient methods currently used by the city of Hamburg with fast and reliable digital methods, saving crucial time in the event of a flood.


## How to start the project

### Nodes

#### Build prerequisities

First, make sure that the riot submodule is downloaded:
```bash
git submodule init
git submodule update
```

To build the firmware, flash, and provision the nodes, the following tools need to be installed
```
make
docker
uv
```

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

### Dashboard

Required dependencies

- docker

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


## Notes for future reference:
Notes from the summer 2026 project group to future contributors.

The working state presented at the summer 2026 presentation can be found at the git tag `presentation-summer-2026`
