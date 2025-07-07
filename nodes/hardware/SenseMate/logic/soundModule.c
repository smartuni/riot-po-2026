#include "soundModule.h"
#include <stdio.h>
#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"


gpio_t sound = GPIO_PIN(0, 8);
gpio_mode_t sound_mode = GPIO_OUT;

void play_sound(int frequency, int duration_ms) {
    // Play sound at a specific frequency for a given duration
    int delay = 1000000 / frequency; // Calculate delay based on frequency
    int cycles = (duration_ms * 1000) / delay; // Calculate number of cycles based on duration

    for (int i = 0; i < cycles; i++) {
        gpio_toggle(sound);
        ztimer_sleep(ZTIMER_USEC, delay / 2); // Half period for toggle
    }
    gpio_clear(sound); // Ensure sound is off after playing
}

void init_sound_module(void) {
    // Initialize the sound module
    gpio_init(sound, sound_mode);
    gpio_clear(sound);
}

void downlink_reveived_sound(void) {
    // Play sound for downlink received
    play_sound(988, 400); // h''
    play_sound(880, 400);  // a''
    play_sound(784, 500);  // g''
    gpio_clear(sound);
}

void uplink_sent_sound(void) {
    // Play sound for uplink sent
    play_sound(784, 300);  // g''
    play_sound(880, 300);  // a''
    play_sound(988, 300); // h''
    gpio_clear(sound);
}

void ble_reveived_sound(void) {
    // Play sound for BLE received
    play_sound(932, 300); // b'' 
    play_sound(831, 400); // as''                           
    
    gpio_clear(sound);
}

void ble_sent_sound(void) {
    // Play sound for BLE sent
    play_sound(831, 400); // as''
    play_sound(932, 300); // b'' 
    gpio_clear(sound);
}



void startup_sound(void){
    play_sound(1318, 500); // e'''
    play_sound(659, 500);  // e''
    play_sound(988, 500);  // h''
    play_sound(880, 500);  // a''
    play_sound(1318, 600); // e'''
    play_sound(988, 800);  // h''

    gpio_clear(sound); // Ensure sound is off after startup sound
}