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

Before flashing make sure [signature keys](../custom-modules/key-distro/README.md) are generated! 

#### Testing 

```make all flash RIOT_CONFIG_USE_TEST=1```

#### Production

```make all flash RIOT_CONFIG_DEVICE_ID=<GATE_ID>```
