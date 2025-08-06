# SenseMate / GateMate LoRaWAN Module

This module provides a LoRaWAN communication interface for **SenseMate** and **GateMate** devices. It enables periodic and event based data serialization, transmission to a backend server, reception of job/target tables, using The Things Network (TTN).

---

## Features

- OTAA (Over-The-Air Activation) join procedure with retry
- Periodic data transmission using CBOR serialization
- LoRaWAN packet handling via RIOT OS `gnrc` networking stack
- Modular support for multiple data sources (`is_state_table`, `seen_status_table`)
- Extensible for additional table/event types
- LoRaWAN interface detection and automatic join
- Integrated with RIOT's `event` and `ztimer` modules

---

## File Structure

```text
mate_lorawan.c        # Main implementation
mate_lorawan.h        # Header declarations (not shown here)
include/              # Additional headers (e.g., event creation)

--

## Known Errors

- Issue while receiving downlinks SenseMate and GateMate are crashing, presumably due to priority management of used threads.

## User Guide

- To start the module use the start_lorawan() method.

- To register a new application and new devices use the following steps:

1. With your browser access The Things Network at [https://www.thethingsnetwork.org/](https://www.thethingsnetwork.org/),
and create an account for yourself by clicking on the "Sign Up" button.

2. Once you have an account, navigate to the console at [https://console.cloud.thethings.network/](https://console.cloud.thethings.network/), and choose the "Europe 1" cluster.

3. On your console, start the creation of a new application, by clicking "Create application".

4. Set your application ID and description, then confirm with "Create application".

5. From the panel that shows your application overview, click on "+ Register end device", inside the "End devices" section.

6. To register the device, choose to "Enter end device specifics manually", so we can provide the configuration. Follow this configuration, make sure to "Show advanced activation, LoRaWAN class and cluster settings":

    | Parameter | Value |
    | --------- | ----- |
    | Frequency plan | "Europe 863-870 MHz (SF9 for RX2 - recommended)" |
    | LoRaWAN version | "LoRaWAN Specification 1.0.3" |
    | Regional Parameters version | "RP001 Regional Parameters 1.0.3 revision A" |
    | Activation mode | "Over the air activation (OTTA)" |
    | Additional LoRaWAN class capabilities | "None (class A only)" |
    | Use network's default MAC settings | True |
    | Join EUI | `00 00 00 00 00 00 00 00` |

   

7. Once you entered the Join EUI, click on "Confirm". Generate the DevEUI and AppKey by clicking on "Generate".

8. Enter a unique name for your device, and click on "Register end device"


