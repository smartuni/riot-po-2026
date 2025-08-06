# RIOT im Internet of Things, Sommersemester 2025

Welcome to the primary repository of the RIOT im Internet of Things project for the Summer 2025 semester at HAW Hamburg. The project was concerned with designing, implementing and presenting technologies to improve the city of Hamburg's methods of managing its harbor's floodgates.

## The Current Situation

Currently, the methods for ensuring floodgates are closed in the event of a flood are inefficient. Field workers receive orders, send updates and otherwise communicate with the central office via walkie-talkies. The central office relies on pen-and-paper record-keeping to log events, such as gates being opened or closed.

## Our Proposed Solution

The solution proposed and developed by our team is as follows: 
<ul>
  <li>Floodgates will be fitted with sensors, called "GateMates," which autonomously report the state of the gate (open or closed) to a server located in the central office.</li>
  <li>Field workers will be issued devices, called "SenseMates," which receive orders from the central office. SenseMates will also record the state of nearby gates by communicating with the GateMates, enabling workers to confirm or refute the state reported by the GateMate.</li>
  <li>The workers at the office will be given a web app with a user interface, allowing them to track the status of gates in real time, as well as providing a digital record of the status of gates and allowing them to issue orders without the need for walkie-talkie communication.</li>
</ul>

Our proposed solution replaces the inefficient methods currently used by the city of Hamburg with fast and reliable digital methods, saving crucial time in the event of a flood.


## How to start the project

### SenseGate

- Connect to Power
- Wait until blue LED is on
- Red LED indicates if door is open (LED off) or closed (LED on)
- Blue LED indicates, if a state update is in progress (LED off). 
- There is a time runoff, which is triggered/restarted on any sensor input. When blue LED is on, the update is processed and sent via LoraWAN.    

#### Deployment

Before flashing make sure [signature keys](nodes/firmware/custom-modules/key-distro/README.md) are generated! 

#### Testing 

```make all flash RIOT_CONFIG_USE_TEST=1```

#### Production

```make all flash RIOT_CONFIG_DEVICE_ID=<GATE_ID>```

### SenseMate

#### Startup
- Use power switch
- Wait until screen is ready

#### Deployment

Before flashing make sure [signature keys](nodes/firmware/custom-modules/key-distro/README.md) are generated! 

#### Testing 

```make all flash RIOT_CONFIG_USE_TEST=1```

#### Production

```make all flash RIOT_CONFIG_DEVICE_ID=<GATE_ID>```

### How to start frontend and backend


#### Requirements to Start the Frontend
- nodejs installed
- npm installed


#### Frontend StartUp
[frontend StartUp](server/frontend/README.md)

#### Backend StartUp
[backend StartUp](server/backend/README.md)

## Nodes

### Overview <div id='overview'/>

SenseMate <div id='overview-sensemate'/>

- node carried around by workers
- receive from and propagate data to other nodes via BLE-module
- receive from and send data to server via LoRa-module


SenseGate <div id='overview-sensegate'/>

- node attached to flood gates
- receive from and propagate data to other nodes via BLE-module
- receive from and send data to server via LoRa-module


Server <div id='overview-server'/>

- receive from and send data to nodes via LoRa-Module
- display real-time accumulated data

## Used Hardware <div id='used-hardware'/>

- Board: Nordic nRF52840
- Power Switch​
- Battery​
- Thumbwheel​
- Navigation​
- Display​
- Soundbuzzer​
- Vibrationmotor​

**SenseMate** is a battery-powered IoT device for monitoring and controlling floodgates. It communicates over BLE and LoRaWAN and provides tactile, visual, and acoustic feedback.

## Key Features <div id='key-features'/>

- **MCU**: [Adafruit Feather nRF52840 Sense](https://www.digikey.de/de/products/detail/adafruit-industries-llc/4516/11684829) (nRF52840 with BLE)
- **LoRa Module**: [Adafruit RFM95W](https://www.digikey.de/de/products/detail/adafruit-industries-llc/3231/6193593)
- **LoRa Antenna**: [Molex 2111400100](https://www.digikey.de/de/products/detail/molex/2111400100/9953925)
- **OLED Display**: [AZDelivery 0.96" SSD1306 I2C](https://www.amazon.de/AZDelivery-Display-Arduino-Raspberry-gratis/dp/B074NJMPYJ?th=1)
- **Thumbwheel Switch**: [SparkFun COM-08184](https://www.digikey.de/de/products/detail/sparkfun-electronics/08184/8543391)
- **Buzzer**: [TDK PS1240P02BT](https://www.digikey.de/de/products/detail/tdk-corporation/PS1240P02BT/935924) (connected to A2)
- **Vibration Motor**: [Seeed 316040004](https://www.digikey.de/de/products/detail/seeed-technology-co-ltd/316040004/5487673) (via NPN + GPIO D3)
- **Power Switch**: [C&K OS102011MA1QN1](https://www.digikey.de/de/products/detail/c-k/OS102011MA1QN1/1981430)
- **Battery**: [Amazon LiPo 1S 450mAh](https://www.amazon.de/dp/B0C5LD55HN) 

## Pinout Summary <div id='pin-summary'/>

| Peripheral         | Connection Type   | MCU Pin       |
|--------------------|-------------------|----------------|
| LoRa SPI           | SPI               | SCK / MOSI / MISO |
| LoRa CS            | GPIO              | D10            |
| LoRa RST           | GPIO              | D9             |
| LoRa IRQ (DIO0)    | GPIO              | D6             |
| LoRa DIO1 / DIO2   | GPIO              | D5 / D4        |
| OLED Display       | I²C               | SDA / SCL      |
| Thumbwheel         | Digital Inputs    | A0 / A1 / A3   |
| Buzzer             | PWM/GPIO          | A2             |
| Vibration Motor    | GPIO (via NPN)    | D3             |
| Battery Voltage    | Analog Input      | A6             |


---

This document is intended for firmware developers. All components are prewired—just refer to the pinout and RIOT board support to begin implementing logic.