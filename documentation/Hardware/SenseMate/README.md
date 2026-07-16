# SenseMate — Hardware Documentation

The **SenseMate** is a custom 4-layer handheld PCB integrating the XIAO nRF52840 Sense Plus MCU module, a LoRa transceiver, an OLED display, haptic feedback (buzzer + vibration motor), a 5-way navigation switch, and an SD card slot.

- **PCB Design:** KiCad 10.0, 4-layer FR4, 0.127 mm min track
- **Gerbers:** `nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/gerber_v1_1/` (13 files, ready for fabrication)
- **Schematic:** `nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/SenseMatePCB.kicad_sch`
- **Layout:** `nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/SenseMatePCB.kicad_pcb`
- **Parts material:** `nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/Parts_Material/`
  - LoRa module datasheets: `Material_LoRa127X-C1/`
  - XIAO/nRF52840 documentation: `Material_nRF52840SensePlus/`

---

## Component List

| Component |  Qty | Purpose | Links |
|-----------|-----|---------|--------|
| **MCU:** Seeed Studio XIAO nRF52840 Sense Plus | 1 | Main MCU — BLE 5.0, Cortex-M4F, 1 MB flash / 256 KB RAM | [Documentation](https://wiki.seeedstudio.com/XIAO_BLE/#xiao-nrf52840-sense-plus-front) [Buy](https://www.seeedstudio.com/Seeed-Studio-XIAO-nRF52840-Sense-Plus-p-6360.html) |
| **LoRa Module:** RFM95W-868S2 - LoRa1276-C1-868 | 1 | 868/915 MHz long-range radio, SPI interface, SX1276-based | [Documentation](https://www.nicerf.com/lora-module/long-range-lora-module-lora1276-c1.html) |
| **OLED display:** AZDelivery 0.96" SSD1306 I2C** (0.96", 128×64) |1 | Monochrome display, I2C address `0x3C` | [More information](https://www.az-delivery.de/products/0-96zolldisplay?srsltid=AfmBOopmjYmHEVXjCJ2GxYgkisQkZ_ro8qa3PN3OuKhFFzLbPdrgE01Q) [Buy](https://www.amazon.de/AZDelivery-Display-Arduino-Raspberry-gratis/dp/B074NJMPYJ?th=1) |
| **Thumbwheel Switch:** SparkFun COM-08184 | 1 | Thumbwheel / joystick for UI navigation (UP, DOWN, SELECT) | [Buy](https://www.digikey.de/de/products/detail/sparkfun-electronics/08184/8543391)  |
| **Buzzer:** TDK PS1240P02BT | 1 | Audio feedback, PWM-driven via SS8050 NPN transistor on P0.02 | [Buy](https://www.digikey.de/de/products/detail/tdk-corporation/PS1240P02BT/935924)  |
| **Vibration motor:** Seeed 316040004 | 1 | Haptic feedback, GPIO-driven via SS8050 NPN transistor on P0.10 | [Buy](https://www.digikey.de/de/products/detail/seeed-technology-co-ltd/316040004/5487673) |
| **SD card slot:** | 1 | SPI interface for data logging (SPI1) |
| **Power Switch** (C&K OS102011MA1QN1) | 1 | Power on/off | [Buy](https://www.digikey.de/de/products/detail/c-k/OS102011MA1QN1/1981430) |
| **Pogo-pin connector** | 1 | Docking station: UART + SWD programming + battery charging |
| **Battery**:  LiPo 1S 450mAh | 1 | Power source via JST connector | [Buy](https://www.amazon.de/dp/B0C5LD55HN) |
| **Transistor:** Y1 SS8050 NPN transistor |2 | Buzzer + vibration motor drivers (SOT-23, 1.5 A Ic) |
| **Diode 1N4148** | 1 | Signal protection |


Detailed firmware documentation: [`documentation/Firmware/SenseMate/`](../Firmware/SenseMate/README.md).

---

## Pin Mapping

All 28 pins of the XIAO nRF52840 Sense Plus module are used. Mapping verified against firmware source code and schematic:

### I/O & Actuators

| MCU Pin | Arduino | Function (SenseMate) | Connected To | Firmware Module | Firmware Reference |
|---------|---------|---------------------|--------------|-----------------|-------------------|
| **P0.02** | A0/D0 | **Buzzer PWM** (BUZ_EN) | Piezo buzzer via SS8050 | `sound.c` | [`main.c:48-52`](../../nodes/firmware/applications/senseMate/main.c#L48) — `PWM_DEV(0)`, channel 0 |
| **P0.03** | A1/D1 | **LoRa RST** (reset) | LoRa module RST | `mate_lorawan` | Schematic net `lora_rst` |
| **P0.04** | A4/D4 | **I2C SDA** | SSD1306 OLED display | `sensemate_ui` / LVGL | [`periph_conf.h: I2C_DEV(0)`](../../nodes/firmware/RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h) |
| **P0.05** | A5/D5 | **I2C SCL** | SSD1306 OLED display | `sensemate_ui` / LVGL | same as above |
| **P0.09** | D14 | **Thumbwheel DOWN** (TMB2) | Nav switch DOWN contact | `sensemate_ui.c` | [`sensemate_ui.c:42`](../../nodes/firmware/applications/senseMate/sensemate_ui/sensemate_ui.c#L42) |
| **P0.10** | D15 | **Vibration motor** (VIB_EN) | Vibration motor via SS8050 | `vibrationModule.c` | [`vibrationModule.c:11`](../../nodes/firmware/applications/senseMate/vibrationModule.c#L11) |
| **P0.15** | D11 | **LoRa DIO0** (interrupt) | LoRa module DIO0 | `mate_lorawan` | Schematic net `lora_dio0` |
| **P0.19** | D12 | **Thumbwheel UP** (TMB1) | Nav switch UP contact | `sensemate_ui.c` | [`sensemate_ui.c:44`](../../nodes/firmware/applications/senseMate/sensemate_ui/sensemate_ui.c#L44) |
| **P0.28** | A2/D2 | **LoRa DIO1** | LoRa module DIO1 | `mate_lorawan` | Schematic net `lora_dio1` |
| **P0.29** | A3/D3 | **LoRa DIO2** | LoRa module DIO2 | `mate_lorawan` | Schematic net `lora_dio2` |
| **P0.31** | D16 | **Battery voltage monitor** (VBAT_MON) | Voltage divider from VBAT | `tables` / system | ADC input for battery status |
| **P1.01** | D13 | **Thumbwheel SELECT** (TMBT1) | Nav switch SELECT contact | `sensemate_ui.c` | [`sensemate_ui.c:43`](../../nodes/firmware/applications/senseMate/sensemate_ui/sensemate_ui.c#L43) |

### Communication Interfaces

| MCU Pin | Arduino | Function | Connected To | Firmware Module | Firmware Reference |
|---------|---------|----------|-------------|-----------------|-------------------|
| **P1.11** | D6/TX | **UART TX** | Pogo pin (docking station) | `periph_uart` | [`periph_conf.h`](../../nodes/firmware/RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h) |
| **P1.12** | D7/RX | **UART RX** | Pogo pin (docking station) | `periph_uart` | same as above |
| **P1.13** | D8/SCK | **SPI SCK** (LoRa) | LoRa module SCK | `mate_lorawan` | [`periph_conf.h: SPI_DEV(0)`](../../nodes/firmware/RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h) |
| **P1.14** | D9/MISO | **SPI MISO** (LoRa) | LoRa module MISO | `mate_lorawan` | same as above |
| **P1.15** | D10/MOSI | **SPI MOSI** (LoRa) | LoRa module MOSI | `mate_lorawan` | same as above |
| **P1.03** | D17/SCK1 | **SPI1 SCK** (SD card) | SD card slot SCK | — | Schematic net `spi_miso1` / `spi_mosi1` / `spi_sck1` |
| **P1.05** | D18/MISO1 | **SPI1 MISO** (SD card) | SD card slot MISO | — | same as above |
| **P1.07** | D19/MOSI1 | **SPI1 MOSI** (SD card) | SD card slot MOSI | — | same as above |

### Power & Programming

| Pin | Function | Connected To |
|-----|----------|-------------|
| **3V3** | 3.3 V power output | All board components |
| **VBUS** | USB power (5 V) | USB connector, docking station VBUS |
| **VBAT** | Battery input | LiPo battery + terminal (via power switch) |
| **GND** | Ground | All grounds, shield |
| **SWDIO** | Programming data | Pogo pin / SWD header |
| **SWDCLK** | Programming clock | Pogo pin / SWD header |
| **RST** | Reset | Reset button, pogo pin |

> **Note:** The v2 design (GERBER v1.1) updated from the legacy **Adafruit Feather nRF52840 Sense** to the **Seeed Studio XIAO nRF52840 Sense Plus**. Pin reassignments are documented as text annotations in the schematic. The firmware supports both boards via conditional compilation (`#ifdef BOARD_ADAFRUIT_FEATHER_NRF52840_SENSE` and `#ifdef BOARD_SEEEDSTUDIO_XIAO_NRF52840_SENSE` in `main.c`, `sensemate_ui.c`, `sound.c`, and `vibrationModule.c`).

---

## Peripherals Used from RIOT

| Peripheral | Config File | Purpose in SenseMate |
|-----------|-------------|----------------------|
| **PWM** | `periph_conf.h` (NRF_PWM0, P0.02 + P0.10) | Buzzer tone generation (`SOUND_PWM_DEV = PWM_DEV(0)`, channel 0) |
| **GPIO** | `periph_conf.h` | Thumbwheel inputs, vibration motor, LoRa control |
| **GPIO IRQ** | `periph_conf.h` | Thumbwheel button interrupts (LVGL input) |
| **I2C** | `periph_conf.h` (I2C_DEV(0): P0.04 SDA, P0.05 SCL) | OLED display communication |
| **SPI** | `periph_conf.h` (SPI_DEV(0): P1.13–P1.15) | LoRaWAN module |
| **UART** | `periph_conf.h` (P1.11 TX, P1.12 RX) | Debug console, shell, pogo UART |
| **ADC** | `arduino_iomap.h` | Battery voltage monitoring (AIN7 / P0.31) |

---

## Links

- **Firmware documentation:** [`documentation/Firmware/SenseMate/`](../Firmware/SenseMate/README.md)
- **Firmware source:** [`nodes/firmware/applications/senseMate/`](../../nodes/firmware/applications/senseMate/)
- **Hardware design files:** [`nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/`](../../nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/)
- **Parts & datasheets:** [`nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/Parts_Material/`](../../nodes/hardware/SenseMate/SenseMatePCB/SenseMate_v2/Parts_Material/)
- **RIOT board config:** [`nodes/firmware/RIOT/boards/seeedstudio-xiao-nrf52840-sense/`](../../nodes/firmware/RIOT/boards/seeedstudio-xiao-nrf52840-sense/)
