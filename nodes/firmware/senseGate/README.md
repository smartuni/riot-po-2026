# SenseGate

### Description
SenseGate is a device which detects the status (open/close) of a flood gate.

### Startup and operation

Pins:
Sensor needs to be connected to: Grnd / A5

- Connect to Power
- Wait until blue LED is on
- Red LED indicates if door is open (LED off) or closed (LED on)
- Blue LED indicates, if a state update is in progress (LED off). 
- There is a time runoff, which is triggered/restarted on any sensor input. When blue LED is on, the update is processed and sent via LoraWAN.    

### Deployment

Before flashing make sure [signature keys](../custom-modules/key-distro/README.md) are generated! 

#### Testing 

```make all flash RIOT_CONFIG_USE_TEST=1```

#### Production

```make all flash RIOT_CONFIG_DEVICE_ID=<GATE_ID>```
