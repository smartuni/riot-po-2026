# Description

The SenseMate project is an IoT solution to monitoring flood gates in the Hamburg harbour area, planned and implemented by the HAW RIOT project team 2025.

It consists of physical devices / nodes (coined SenseMate and SenseGate respectively) and web frontend.

## Overview

SenseMate

- receive from and propagate data to other nodes via BLE-module
- receive from and send data to server via LoRa-module
- node carried around by workers
- To flash enter nodes/hardware/SenseMate/logic

```sh
$ make all flash term
```

SenseGate

- receive from and propagate data to other nodes via BLE-module
- receive from and send data to server via LoRa-module
- node attached to flood gates
- To flash enter nodes/hardware/GateMate/logic

```sh
$ make all flash term
```

Server

- receive from and send data to nodes via LoRa-Module
- display real-time accumulated data

## Used Hardware

- Board: Nordic nRF52840
- Power Switch​
- Battery​
- Thumbwheel​
- Navigation​
- Display​
- Soundbuzzer​
- Vibrationmotor​

## GitFlow

See GitFlow here [GitFlow Document](/.github/GitFlow)

# Init git submodlues (RIOT) with
To init the submodules run submodules_linux.sh or submodules_windows.bat
You can also run these commands:
```
git submodule init
git submodule update
```
