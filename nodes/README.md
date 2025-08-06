# Description

This is the node directory of the project. It contains all necessary modules and functions of the SenseMate and SenseGate node.

# Overview

## Custom Modules

custom-modules takes from the available RIOT modules and adapts these to suit the SenseMate project's needs.

These include:
- COSE for CBOR signing
- KEY-DISTRO for secure data signing and verification
- BLE for node-to-node communication via Bluetooth
- LORAWAN for server-to-node and node-to-server communication via LoRaWan
- TABLES to manage state tables and encode them to and decode them from CBOR

## Hardware

SenseMate hardware components and drawings 

## Firmware

SenseMate and SenseGate source code

### Deployment

`make all flash` or with terminal output `make all flash term` TODO