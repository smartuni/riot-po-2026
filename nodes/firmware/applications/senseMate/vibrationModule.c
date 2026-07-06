#include "include/vibrationModule.h"
#include <stdio.h>
#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"

#define SOUND_PWM_DEV PWM_DEV(0)
#if defined BOARD_ADAFRUIT_FEATHER_NRF52840_SENSE // v1 board
gpio_t vibration = GPIO_PIN(1, 9);
#elif defined BOARD_SEEEDSTUDIO_XIAO_NRF52840_SENSE // v2 board
gpio_t vibration = GPIO_PIN(0, 10);
#else
#error "Missing board-specific configuration."
#endif
gpio_mode_t vibtation_mode = GPIO_OUT;

void init_vibration_module(void) {
    // Initialize the vibration module
    gpio_init(vibration, vibtation_mode);
    gpio_clear(vibration);
}

void start_vibration(void) {
    // Start vibration
    gpio_set(vibration);
}

void stop_vibration(void) {
    // Stop vibration
    gpio_clear(vibration);
}
