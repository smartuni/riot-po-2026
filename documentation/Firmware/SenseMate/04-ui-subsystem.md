# 04 — UI Subsystem

## Overview

SenseMate has a monochrome **128x64 pixel OLED display** (SSD1306 driver) controlled by the **LVGL (Light and Versatile Graphics Library)**. User input comes from a **3-button thumbwheel** (up, down, select). Feedback is provided through a **buzzer** (audio) and a **vibration motor** (haptic).

## Display Hardware

The SSD1306 OLED connects to the nRF52840 via the I2C bus:

```c
// nodes/firmware/applications/senseMate/Makefile:55-57
CFLAGS += -DI2C_DEVICE_IDX=0
CFLAGS += -DI2C_DISPLAY_ADDR=0x3c
CFLAGS += -DU8G2_DISPLAY_INIT_FUNC=u8g2_Setup_ssd1306_i2c_128x64_noname_f
```

- **I2C bus**: Index 0
- **Display address**: `0x3c` (standard for SSD1306)
- **Resolution**: 128 x 64 pixels
- **Color depth**: 1 bit per pixel (monochrome) — configured in `lv_conf.h:41` when `MODULE_U8G2_DISP_DEV` is active

The RIOT `u8g2` package bridges the LVGL graphics library to the SSD1306 display driver.

## LVGL Configuration

The file `nodes/firmware/applications/senseMate/lv_conf.h` configures LVGL for this device. Key settings:

| Setting | Value | Purpose |
|---------|-------|---------|
| `LV_COLOR_DEPTH` | `1` (when using u8g2) | Monochrome — saves RAM |
| `LV_MEM_SIZE` | `16 KB` (set in Makefile) | LVGL's internal memory pool |
| `LV_DPI_DEF` | `130` | Dots per inch for widget sizing |
| Theme | `lv_theme_mono` | Monochrome-optimized theme |
| Widgets enabled | slider, chart, win, list, tileview | UI building blocks |
| Layout | `LV_USE_FLEX` | Flexbox-like layout system |
| Fonts | Montserrat 8/10/12/14/16, unscii 8/16 | System fonts |

## UI Architecture

The UI runs in its own **separate RIOT thread** to avoid blocking the main application loop:

```c
// nodes/firmware/applications/senseMate/sensemate_ui/sensemate_ui.c:884-888
kernel_pid_t ui_pid = thread_create(_ui_thread_stack, sizeof(_ui_thread_stack),
                                    THREAD_PRIORITY_MAIN - 2,
                                    THREAD_CREATE_STACKTEST, _ui_thread, NULL,
                                    "ui");
```

### How the UI Updates

The UI uses a "pull" model with a shared state struct:

```c
// nodes/firmware/applications/senseMate/include/sensemate_ui.h:63-71
typedef struct {
    uint16_t visible_gate_cnt;
    uint16_t pending_jobs_cnt;
    uint16_t visible_mate_cnt;
    ui_connection_state_t lora_state;
    ui_connection_state_t ble_state;
    ui_connection_state_t usb_state;
    bool pending_notifications;
} ui_data_t;
```

The main loop updates this struct every second (`main.c:306-337`) and calls `sensemate_ui_update()` when values change. The UI thread reads the struct and redraws affected widgets.

### Connection State Indicators

Each communication channel (BLE, LoRaWAN, USB) has a visual state indicator in the header bar with animation support (`sensemate_ui.c:139-155`):

| State | Visual Behavior |
|-------|----------------|
| `DISCONNECTED` | Hidden |
| `ESTABLISHING_CONNECTION` | Blinks on/off every 500ms |
| `CONNECTED` | Shown continuously |
| `RECEIVED` | Shows a down-arrow icon briefly, then returns to CONNECTED |
| `TRANSMITTED` | Shows an up-arrow icon briefly, then returns to CONNECTED |

### Callback Interface

The UI is decoupled from data sources through callbacks (`sensemate_ui.h:55-61`):

```c
typedef struct {
    ui_data_element_iter_cb_t all_gates_iter;    // Iterate over known gates
    ui_data_element_iter_cb_t jobs_iter;          // Iterate over pending jobs
    ui_create_data_element_cb_t put_gate_observation; // Record worker observation
    ui_set_min_visible_rssi_cb_t set_min_visible_rssi; // Filter threshold
    ui_get_min_visible_rssi_cb_t get_min_visible_rssi; // Read filter threshold
} ui_data_cbs_t;
```

These callbacks are provided by `main.c:216-222` and bridge the UI to the `tables` module.

### Screen Layout

The UI uses a **tileview** (horizontally scrollable panels) with two main screens:

```
┌─────────────────────────────────────┐
│ [BLE ][LoRa][      Mate-3 ][USB][BAT]│  ← header bar with connection states
├───────────────┬─────────────────────┤
│               │                     │
│  DASHBOARD    │    MENU LIST        │  ← tiles (swipe left/right)
│               │                     │
│ [Gate] [Jobs] │  Closeby Gates      │
│   3      0    │  All Gates          │
│ [Person]      │  Jobs               │
│   1           │  Messages           │
│               │  Settings           │
├───────────────┴─────────────────────┤
│  SUBMENU TILE                       │  ← dynamically loaded submenus
│  (gate lists, settings, prompts)    │
└─────────────────────────────────────┘
```

### Screens (in `sensemate_ui.c`)

1. **Dashboard** (`_create_dashboard`, line 487) — Shows at-a-glance info: gate count badge, pending jobs badge, nearby persons count badge.
2. **Main Menu** (`_create_list_menu`, line 345) — Navigate to: Closeby Gates, All Gates, Jobs, Messages, Settings.
3. **Gate Lists** (`_create_gate_list`, line 376) — Lists all or nearby gates with open/closed lock icons. Each gate can be tapped to open a dialog for reporting a different observed state.
4. **Settings** (`_settings_menu_dyn_enter`, line 686) — Bluetooth settings and a minimum RSSI slider (range -100 to -30 dBm) that filters which devices appear as "visible."

### Custom Icons & Fonts

The `sensemate_ui/` directory contains bitmap icons converted from SVG to C arrays:

| Icon File | Size | Purpose |
|-----------|------|---------|
| `gate_icon_32x32.c` | 32x32 | Gate icon for the dashboard |
| `gate_icon_open_8x9.c` | 8x9 | Open lock (green/open gate indicator) |
| `gate_icon_closed_8x9.c` | 8x9 | Closed lock (red/closed gate indicator) |
| `lock_open_icon_16x16.c` | 16x16 | Open padlock in gate lists |
| `lock_closed_icon_16x16.c` | 16x16 | Closed padlock in gate lists |
| `tasks_icon_25x32.c` | 25x32 | Tasks/jobs icon for the dashboard |
| `person_icon_32x32.c` | 32x32 | Person icon for nearby mates |
| `bluetooth_icon_7x11.c` | 7x11 | Bluetooth logo in header |
| `arrow_up_icon_5x9.c` | 5x9 | Up arrow (transmit indicator) |
| `arrow_down_icon_5x9.c` | 5x9 | Down arrow (receive indicator) |
| `arrow_up_down_icon_5x9.c` | 5x9 | Up+down arrow (idle indicator) |

Custom fonts: `helvetica_light_12`, `helvetica10`, `font_goldfish`, `micro_regular`, `cnc` (converted from TTF with LVGL's font converter).

## Sound Subsystem

### Low-Level PWM Buzzer (`sound.c`)

The buzzer is connected to a **PWM output** pin. PWM (Pulse Width Modulation) generates audio tones by rapidly switching the pin on and off at a target frequency:

```c
// nodes/firmware/applications/senseMate/main.c:48-55
#define SOUND_PWM_DEV PWM_DEV(0)
// v1 board: PWM channel 1
// v2 board: PWM channel 0 (GPIO_PIN(0, 8))
```

**How tones work** (`sound.c:87-115`): When `sound_init()` is called, it tries to match each desired frequency (e.g., 988 Hz) to the closest achievable PWM frequency by iterating through possible resolution values. The matched settings are stored so playback is fast.

**How playback works** (`sound.c:133-148`): For each beep in a sequence, the PWM is configured for that frequency, driven at 50% duty cycle (half the resolution value), held for the beep's duration, then turned off. A frequency of 0 Hz means "silence/pause."

### Predefined Sounds

Seven sound sequences are defined (`sound.c:24-75`):

| Sequence | Description | Used For |
|----------|-------------|----------|
| `startup_beep_seq` | Rising melody: E'', E', H'', A'', E'' | Device boot |
| `downlink_rx_seq` | Descending trio: H'', A'', G'' | LoRaWAN downlink received |
| `uplink_tx_seq` | Ascending trio: G'', A'', H'' | LoRaWAN uplink sent |
| `ble_rx_seq` | Two quick notes | BLE data received |
| `ble_tx_seq` | Two notes | BLE data transmitted |
| `tables_news_seq` | Three short beeps | New record merged into tables |
| `closeby_todo_seq` | Alternating F''-A'' pattern | Alert for nearby pending job |

### Sound Module (`soundModule.c`)

A higher-level, event-driven wrapper for sound playback. Benefits over `sound.c`:

- Uses RIOT's **event system** — other threads post events to a queue, and the sound thread plays them in order
- Includes a simpler **GPIO-based** tone generator (`play_sound()` function) as an alternative to PWM
- Coordinates sound with vibration (e.g., a LoRaWAN downlink triggers both sound + vibration)

```c
// nodes/firmware/applications/senseMate/soundModule.c:19-29
void play_sound(int frequency, int duration_ms) {
    int delay = 1000000 / frequency;
    int cycles = (duration_ms * 1000) / delay;
    for (int i = 0; i < cycles; i++) {
        gpio_toggle(sound);              // GPIO_PIN(0, 8)
        ztimer_spin(ZTIMER_USEC, delay / 2);
    }
    gpio_clear(sound);
}
```

### Sound Thread

```c
// nodes/firmware/applications/senseMate/sound.c:173-194
static void* sound_thread(void *arg) {
    // ... message queue setup ...
    while(1) {
        msg_receive(&m);
        // Play sound (async or blocking)
    }
}
```

The blocking mode (`sound_play_blocking`) is useful when the caller needs to wait until playback finishes (e.g., coordinating sound with vibration).

## Vibration Motor (`vibrationModule.c`)

A simple GPIO-controlled vibration motor on pin `GPIO_PIN(0, 10)` (v2 board):

```c
// nodes/firmware/applications/senseMate/vibrationModule.c:10-14
gpio_t vibration = GPIO_PIN(0, 10); // v2 board
// v1 board: GPIO_PIN(1, 9)
```

Three functions:
- `init_vibration_module()` — initialize GPIO as output, set low
- `start_vibration()` — set GPIO high (motor on)
- `stop_vibration()` — set GPIO low (motor off)

## Event Handling (`events_creation.c`)

The events module runs in its own thread and responds to incoming BLE/LoRaWAN events:

```c
// nodes/firmware/applications/senseMate/events_creation.c:31-36
void init_event(void) {
    thread_create(thread_stack, sizeof(thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, thread_events_function, NULL,
                  "events_creation");
}
```

### Defined Events (`events_creation.h:20-35`)

| Event | Trigger | Handler Behavior |
|-------|---------|-----------------|
| `eventNews` | LoRaWAN downlink received | Vibrate → set LoRa state to RECEIVED → play downlink sound → stop vibration |
| `eventBleNews` | BLE data merged into tables, new info available | Set BLE state to RECEIVED → trigger UI refresh → play tables-new sound |
| `eventBleRx` | Raw BLE data received | Set BLE state to RECEIVED → play BLE receive sound |
| `eventBleTx` | BLE data transmitted | Set BLE state to TRANSMITTED |
| `event_reactivate` | Button debounce timer | Reactivates inputs after debounce period |

The event handlers update the shared `ui_data_t` state and call `sensemate_ui_update()` to refresh the display (or post an `event_trigger_ui_refresh` event to do so asynchronously).

## Thumbwheel Input

A 3-button encoder (up, select, down) serves as the UI input device:

```c
// nodes/firmware/applications/senseMate/sensemate_ui/sensemate_ui.c:41-44
// v2 board:
#define THUMBWHEEL_PIN_DOWN   GPIO_PIN(0, 9)
#define THUMBWHEEL_PIN_SELECT GPIO_PIN(1, 1)
#define THUMBWHEEL_PIN_UP     GPIO_PIN(0, 19)
```

LVGL treats this as an **encoder-type** input device. The read callback (`_encoder_with_keys_read`, line 190) polls the GPIO pins and maps them to LVGL key codes:
- Down pressed → `LV_KEY_RIGHT`
- Select pressed → `LV_KEY_ENTER`
- Up pressed → `LV_KEY_LEFT`

Navigation groups (`lv_group_t`) define which widgets can receive focus, and LVGL cycles focus between them based on these key inputs.
