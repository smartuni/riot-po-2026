#include "soundModule.h"
#include <stdio.h>
#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "thread.h"
#include "vibrationModule.h"
#include "event/timeout.h"
#include "event/thread.h"


gpio_t sound = GPIO_PIN(0, 8);
gpio_mode_t sound_mode = GPIO_OUT;
bool thread_job = false;
bool startup_toplay = false;
bool ble_received_toplay = false;
bool ble_send_toplay = false;
bool uplink_sent_toplay = false;
bool downlink_received_toplay = false;
event_queue_t sound_queue;

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
    play_sound(932, 50); // b'' 
    play_sound(831, 50); // as''                           
    
    gpio_clear(sound);
}

void internal_ble_send(void) {
    // Play sound for BLE sent
    play_sound(831, 400); // as''
    play_sound(932, 300); // b'' 
    gpio_clear(sound);
}

void internal_tables_news(void) {
    //Play sound for tables news
    play_sound(784, 300);  // g''
    play_sound(932, 300); // b'' 
    play_sound(587, 300); // d''
}

void internal_close_by_todo(void) {
    // Play sound for close by todo
    start_vibration();
    play_sound(698, 300);  // f''
    play_sound(880, 300);  // a''
    ztimer_sleep(ZTIMER_MSEC, 500); // Half period for toggle
    play_sound(698, 300);  // f''
    play_sound(880, 300);  // a''
    gpio_clear(sound);
    stop_vibration();
}

static void sound_handler(event_t *event){
    puts("Sound handler called.");
    if(event == &start_sound_event){
        internal_startup();
        puts("Playing startup sound.");
    }
    else if(event == &downlink_sound_event){
        internal_downlink_reveived();
        puts("Playing downlink sound.");
    }
    else if(event == &uplink_sound_event){
        internal_uplink_send();
    }
    else if(event == &ble_received_sound_event){
        internal_ble_received();
    }
    else if(event == &ble_sent_sound_event){
        internal_ble_send();
    }
    else if(event == &tables_news_sound_event){
        internal_tables_news();
    }else if(event == &close_by_todo_sound_event){
        internal_close_by_todo();
    }

    else {
        puts("Unknown sound event received.");
    }
}

event_t start_sound_event = { .handler = sound_handler };
event_t downlink_sound_event = { .handler = sound_handler };
event_t uplink_sound_event = { .handler = sound_handler };
event_t ble_received_sound_event = { .handler = sound_handler };
event_t ble_sent_sound_event = { .handler = sound_handler };
event_t tables_news_sound_event = { .handler = sound_handler };
event_t close_by_todo_sound_event = { .handler = sound_handler };



void* thread_sound_function(void *arg) {
    (void)arg; // Unused argument
    
    event_queue_init(&sound_queue); // Initialize the sound event queue

    while(1){
        puts("Sound thread running.");
        event_loop(&sound_queue);
        //event_t *ev = event_wait(&sound_queue);
        //ev->handler(ev);
        puts("Sound thread event loop finished.");
        /*ztimer_sleep(ZTIMER_MSEC, 500); // Sleep for 1 second
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
        }*/
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
    event_post(&sound_queue, &downlink_sound_event);
    puts("Downlink sound event posted.");
    //downlink_received_toplay = true;
    //thread_job = true;
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
    event_post(&sound_queue, &start_sound_event);
    puts("Startup sound event posted.");
    //startup_toplay = true;
    //thread_job = true;
}