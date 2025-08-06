#include "include/vibrationModule.h"
#include <stdio.h>
#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"

gpio_t vibration = GPIO_PIN(1, 9);
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