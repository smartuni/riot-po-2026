# Description

This is the node directory of the project. It contains all necessary modules and functions of the SenseMat and SenseGate node.

# Overview

## Custom Modules

custom-modules takes from the available RIOT modules and adapts these to suit the SenseMate project's needs.

These include:
- COSE for CBOR encryption
- KEY-DISTRO for secure data encryption and decryption
- BLE for node-to-node communication via Bluetooth
- LORAWAN for server-to-node and node-to-server communication via LoRaWan
- TABLES for CBOR encoding and decoding and processing of internal information

## Hardware

SenseMate hardware components and drawings 

## Firmware

SenseMate and SenseGate source code

### Deployment

`make all flash` or with terminal output `make all flash term` TODO