# SenseGate — Hardware Documentation

The **SenseGate** is based on the SenseMate - A senseMate soldered to the Suerface of a SenseGate PCB with special added components and changes in the firmware. 

The **SenseGate** is a custom 2-layer PCB designed as a sensor interface carrier for the nRF52840 module (Seeed Studio XIAO nRF52840 Sense). It mounts directly on physical flood gates and detects open/closed state via reed switches and an inductive sensor.

- **PCB Design:** KiCad 10.0, 2-layer FR4, 1.6 mm, HASL SnPb finish
- **Gerbers:** `nodes/hardware/SenseGate/SenseGatePCB/SenseGateGerber/` (14 files, ready for fabrication)
- **Schematic:** `nodes/hardware/SenseGate/SenseGatePCB/sensegate_pcb.kicad_sch`
- **Layout:** `nodes/hardware/SenseGate/SenseGatePCB/sensegate_pcb.kicad_pcb`

---

## Component List

| Component | Reference | Qty | Purpose |
|-----------|-----------|-----|---------|
| **nRF52840 MCU** (XIAO module) | J4 (24-pin connector) | 1 | Main controller — runs RIOT OS firmware |
| **Reed switch** (normally open) | SW1–SW4 | 4 | Magnetic gate position sensors (magnet on gate triggers when closed) |
| **Inductive sensor** | — | 1 | Metal proximity sensor, analog output via ADC |
| **DD05CVSA DC-DC converter** | U | 1 | 5V / 3.3V regulated power supply |
| **MT3608 boost converter** | U | 1 | Solar panel voltage step-up |
| **AO3401A P-Channel MOSFET** | Q | 1 | Battery/power switching (SOT-23, −4.0 A, −30 V) |
| **BSS138 N-Channel MOSFET** | Q | 1 | Level shifting (SOT-23, 50 V, 0.22 A) |
| **Potentiometer** | RV | 1 | ADC voltage divider adjustment for inductive sensor threshold |
| **Solar cell** | SC | 1 | Optional solar charging input (monitored via `MON_SOLAR`) |
| **Battery** | BT | 1 | Multiple-cell battery input |
| **DPDT switch** | SW | 1 | Power / mode configuration |

Detailed firmware documentation: [`documentation/Firmware/SenseGate/`](../Firmware/SenseGate/README.md).

---

## Pin Mapping

The MCU connects to the SenseGate carrier via a 24-pin castellation connector (`SenseGateLib.pretty/SenseMateInterface.kicad_mod`). Signal assignments on the carrier board:

| MCU Pin | Function (SenseGate) | Connected To | Firmware Module | Firmware Reference |
|---------|---------------------|--------------|-----------------|-------------------|
| **P0.02** (A0/D0) | ADC reference / sensor bias | SenseGate PCB | — | — |
| **P0.03** (A1/D1) | LoRa RST | LoRa module RST pin | `mate_lorawan` | LoRa module datasheet |
| **P0.04** (A4/D4) | **Sensor power control** (DCDC enable) | Inductive sensor power MOSFET | `inductive_sensor` | [`main.c:56`](../../nodes/firmware/applications/senseGate/main.c#L56) |
| **P0.05** (A5/D5) | Battery monitor ADC | Voltage divider from battery | `inductive_sensor` | `INDUCTIVE_SENSOR_ADC_LINE (4)` — see below |
| **P0.09** (D14) | **Limit switch input** (reed switch) | Reed switch SW1–SW4 (via pull-up) | `gate_observer` | [`main.c:51`](../../nodes/firmware/applications/senseGate/main.c#L51) |
| **P0.10** (D15) | Vibration motor enable | — | *(unused on SenseGate)* | — |
| **P0.15** (D11) | LoRa DIO0 (interrupt) | LoRa module DIO0 pin | `mate_lorawan` | LoRa module datasheet |
| **P0.19** (D12) | Thumbwheel (n.c. on SenseGate) | — | — | — |
| **P0.28** (A2/D2) | LoRa DIO1 | LoRa module DIO1 pin | `mate_lorawan` | LoRa module datasheet |
| **P0.29** (A3/D3) | LoRa DIO2 | LoRa module DIO2 pin | `mate_lorawan` | LoRa module datasheet |
| **P0.31** (D16) | GPIO (spare) | — | — | — |
| **P1.01** (D13) | Thumbwheel touch (n.c. on SenseGate) | — | — | — |
| **P1.11** (D6/TX) | UART TX (debug / pogo) | Pogo pin (programming) | `periph_uart` | [`periph_conf.h: UART0`](../../nodes/firmware/RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h) |
| **P1.12** (D7/RX) | UART RX (debug / pogo) | Pogo pin (programming) | `periph_uart` | same as above |
| **P1.13** (D8/SCK) | SPI SCK | LoRa module SCK | `mate_lorawan` | [`periph_conf.h: SPI_DEV(0)`](../../nodes/firmware/RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h) |
| **P1.14** (D9/MISO) | SPI MISO | LoRa module MISO | `mate_lorawan` | same as above |
| **P1.15** (D10/MOSI) | SPI MOSI | LoRa module MOSI | `mate_lorawan` | same as above |
| **VBAT** | Battery voltage input | Battery + terminal via power path | — | — |
| **3V3** | 3.3 V output (from DD05CVSA) | All board components | — | — |
| **GND** | Ground | All grounds | — | — |
| **SWDIO** | Programming data | Pogo pin / SWD connector | — | — |
| **SWDCLK** | Programming clock | Pogo pin / SWD connector | — | — |

### Inductive Sensor ADC

The inductive sensor ADC line is configured in firmware as:

```c
// nodes/firmware/applications/senseGate/main.c:56-60
#define INDUCTIVE_SENSOR_DCDC_PWR_PIN GPIO_PIN(0,4)  // P0.04 enables sensor power
#define INDUCTIVE_SENSOR_ADC_LINE (4)                 // ADC_LINE(4) = AIN4 = P0.28 (A2)
#define INDUCTIVE_SENSOR_ADC_VREF_MV (3300)           // 3.3 V MCU reference
#define INDUCTIVE_SENSOR_VREF_MV     (11000)           // 11 V external reference
```

The `ADC_LINE(4)` maps to **GPIO pin P0.28** (Arduino A2 / D2) on the Seeed Studio XIAO nRF52840 Sense board.

> **ADC reference voltage:** The MCU's internal ADC reference is 3.3 V. For measuring external sensor voltages up to 11 V, a voltage divider is used (`INDUCTIVE_SENSOR_VREF_MV = 11000`).

### Limit Switch (Reed Switch)

The single limit switch input is on **GPIO_PIN(0, 9)** (XIAO D14 / P0.09):

```c
// nodes/firmware/applications/senseGate/main.c:50-51
#elif defined BOARD_SEEEDSTUDIO_XIAO_NRF52840_SENSE
#define REED_0_PIN_0 GPIO_PIN(0, 9)
```

The SenseGate carrier board connects four reed switches (SW1–SW4) in parallel or via a combiner circuit. The firmware reads one GPIO line.

---

## RIOT OS Board Hierarchy

The `seeedstudio-xiao-nrf52840-sense` board inherits configuration through 4 levels:

```
seeedstudio-xiao-nrf52840-sense  (concrete — adds IMU, 2nd I2C bus)
  └─ common/seeedstudio-xiao-nrf52840  (pins, LEDs, SPI, I2C, UART, PWM, NOR flash)
       └─ common/adafruit-nrf52-bootloader  (bootloader + USB CDC ACM)
            └─ common/nrf52  (CPU = nrf52, RTT, timers, netif)
```

Key configuration files and what they define:

| File | What It Defines |
|------|----------------|
| `RIOT/boards/seeedstudio-xiao-nrf52840-sense/Makefile.features` | CPU_MODEL, requires `periph_gpio_ll`, includes nrf52 features |
| `RIOT/boards/seeedstudio-xiao-nrf52840-sense/board.c` | IMU power-up (P1.08), unique to Sense variant |
| `RIOT/boards/common/seeedstudio-xiao-nrf52840/include/periph_conf.h` | **All peripheral pin mappings** (UART, SPI×2, I2C×2, PWM) |
| `RIOT/boards/common/seeedstudio-xiao-nrf52840/include/board.h` | LED pins (RGB), NOR flash params, IMU params (conditional) |
| `RIOT/boards/common/seeedstudio-xiao-nrf52840/include/arduino_iomap.h` | Arduino D0–D13 + A0–A5 pin mapping, ADC line mapping |
| `RIOT/boards/common/seeedstudio-xiao-nrf52840/include/gpio_params.h` | SAUL GPIO params for RGB LEDs |
| `RIOT/boards/common/seeedstudio-xiao-nrf52840/mtd.c` | External 2 MB P25Q16H NOR flash (SPI_DEV(1)) |

For full details, see `documentation/Firmware/SenseGate/03-architecture.md`.

---

## Peripherals Used from RIOT

| Peripheral | Config File | Purpose in SenseGate |
|-----------|-------------|----------------------|
| **GPIO** | `periph_conf.h` (board level) | Digital I/O: limit switch, sensor power, LoRa control |
| **GPIO IRQ** | `periph_conf.h` | Interrupt on limit switch change |
| **ADC** | `arduino_iomap.h` (ADC_LINE(4)) | Analog reading of inductive sensor |
| **UART** | `periph_conf.h` (P1.11 TX, P1.12 RX) | Debug console, serial shell |
| **SPI** | `periph_conf.h` (SPI_DEV(0) = LoRa, SPI_DEV(1) = NOR flash) | LoRaWAN module, external flash |
| **PWM** | `periph_conf.h` (P0.02, P0.10) | *(unused on SenseGate)* |

---

## Links

- **Firmware documentation:** [`documentation/Firmware/SenseGate/`](../Firmware/SenseGate/README.md)
- **Firmware source:** [`nodes/firmware/applications/senseGate/`](../../nodes/firmware/applications/senseGate/)
- **Hardware design files:** [`nodes/hardware/SenseGate/SenseGatePCB/`](../../nodes/hardware/SenseGate/SenseGatePCB/)
- **RIOT board config:** [`nodes/firmware/RIOT/boards/seeedstudio-xiao-nrf52840-sense/`](../../nodes/firmware/RIOT/boards/seeedstudio-xiao-nrf52840-sense/)
