# SenseMate

### Description
This module provides the Application for the SenseMate Device.
SenseMate is a handheld device that:

- receives from and propagate data to other nodes via BLE-module
- receives from and send data to server via LoRa-module
- is carried around by workers

### Startup
- Use power switch
- Wait until screen is ready

### Deployment

After flashing, use the [identity-manager](../../identity-manager/README.md) to generate a node identity and provision it to the node.

#### Building and Flashing

```make all flash```
