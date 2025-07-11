# SenseMate Hardware Overview

**SenseMate** is a battery-powered IoT device for monitoring and controlling floodgates. It communicates over BLE and LoRaWAN and provides tactile, visual, and acoustic feedback.

## Key Features

- **MCU**: [Adafruit Feather nRF52840 Sense](https://www.digikey.de/de/products/detail/adafruit-industries-llc/4516/11684829) (nRF52840 with BLE)
- **LoRa Module**: [Adafruit RFM95W](https://www.digikey.de/de/products/detail/adafruit-industries-llc/3231/6193593)
- **LoRa Antenna**: [Molex 2111400100](https://www.digikey.de/de/products/detail/molex/2111400100/9953925)
- **OLED Display**: [AZDelivery 0.96" SSD1306 I2C](https://www.amazon.de/AZDelivery-Display-Arduino-Raspberry-gratis/dp/B074NJMPYJ?th=1)
- **Thumbwheel Switch**: [SparkFun COM-08184](https://www.digikey.de/de/products/detail/sparkfun-electronics/08184/8543391)
- **Buzzer**: [TDK PS1240P02BT](https://www.digikey.de/de/products/detail/tdk-corporation/PS1240P02BT/935924) (connected to A2)
- **Vibration Motor**: [Seeed 316040004](https://www.digikey.de/de/products/detail/seeed-technology-co-ltd/316040004/5487673) (via NPN + GPIO D3)
- **Power Switch**: [C&K OS102011MA1QN1](https://www.digikey.de/de/products/detail/c-k/OS102011MA1QN1/1981430)
- **Battery**: [Amazon LiPo 1S 450mAh](https://www.amazon.de/dp/B0C5LD55HN) 

## Pinout Summary

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
