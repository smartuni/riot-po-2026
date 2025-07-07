#include "soundModule.h"
#include <stdio.h>
#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "thread.h"
#include "vibrationModule.h"


gpio_t sound = GPIO_PIN(0, 8);
gpio_mode_t sound_mode = GPIO_OUT;
bool thread_job = false;
bool startup_toplay = false;
bool ble_received_toplay = false;
bool ble_send_toplay = false;
bool uplink_sent_toplay = false;
bool downlink_received_toplay = false;

char thread_stack[THREAD_STACKSIZE_DEFAULT];

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

void internal_startup(void){
    start_vibration();
    play_sound(1318, 500); // e'''
    play_sound(659, 500);  // e''
    play_sound(988, 500);  // h''
    play_sound(880, 500);  // a''
    play_sound(1318, 600); // e'''
    play_sound(988, 800);  // h''

    gpio_clear(sound); // Ensure sound is off after startup sound
    stop_vibration();
}

void internal_downlink_reveived(void) {
    // Play sound for downlink received
    start_vibration();
    play_sound(988, 400); // h''
    play_sound(880, 400);  // a''
    play_sound(784, 500);  // g''
    gpio_clear(sound);
    stop_vibration();
}

void internal_uplink_send(void) {
    // Play sound for uplink sent
    play_sound(784, 300);  // g''
    play_sound(880, 300);  // a''
    play_sound(988, 300); // h''
    gpio_clear(sound);
}

void internal_ble_received(void) {
    // Play sound for BLE received
    start_vibration();
    play_sound(932, 300); // b'' 
    play_sound(831, 400); // as''                           
    
    gpio_clear(sound);
    stop_vibration();
}

void internal_ble_send(void) {
    // Play sound for BLE sent
    play_sound(831, 400); // as''
    play_sound(932, 300); // b'' 
    gpio_clear(sound);
}

void* thread_sound_function(void *arg) {
    (void)arg; // Unused argument
    while(1){
        ztimer_sleep(ZTIMER_MSEC, 500); // Sleep for 1 second
        if(thread_job){
            thread_job = false;
            if(startup_toplay){
                startup_toplay = false;
                internal_startup();
                printf("Playing startup sound.\n");
            }
            if(ble_received_toplay){
                ble_received_toplay = false;
                internal_ble_received();
            }
            if(ble_send_toplay){
                ble_send_toplay = false;
                internal_ble_send();
            }
            if(uplink_sent_toplay){
                uplink_sent_toplay = false;
                internal_uplink_send();
            }
            if(downlink_received_toplay){
                downlink_received_toplay = false;
                internal_downlink_reveived();
            }
        }
    }
}



void init_sound_module(void) {
    // Initialize the sound module
    gpio_init(sound, sound_mode);
    gpio_clear(sound);

    thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, thread_sound_function, NULL, "sound_thread");
    printf("Sound module initialized.\n");
}

void downlink_reveived_sound(void) {
    downlink_received_toplay = true;
    thread_job = true;
}

void uplink_sent_sound(void) {
    uplink_sent_toplay = true;
    thread_job = true;
}

void ble_reveived_sound(void) {
    ble_received_toplay = true;
    thread_job = true;
}

void ble_sent_sound(void) {
    ble_send_toplay = true;
    thread_job = true;
}

void startup_sound(void){
    startup_toplay = true;
    thread_job = true;
}