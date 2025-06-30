# SenseGate

### Description
SenseGate is a device which detects the status (open/close) of a flood gate.

### Flashing

In directory 'logic':
`make all flash` or with terminal output `make all flash term`

### Startup and operation

Pins:
Sensor needs to be connected to: Grnd / A5

- Connect to Power
- Wait until blue LED is on
- Red LED indicates if door is open (LED off) or closed (LED on)
- Blue LED indicates, if a state update is in progress (LED off). 
- There is a time runoff, which is triggered/restarted on any sensor input. When blue LED is on, the update is processed and sent via LoraWAN.    

### Change GATE_ID and Debounce Time
- Constants/ Macros can be found in folder `logic/header/event_creation.h`
