#include "header/detectDoorStatus.h"

#define TRIGGER GPIO_PIN(0, 3) // PIN A5
//#define TRIGGER GPIO_PIN(1, 2) // Test Button

gpio_t led1 = GPIO_PIN(1, 9);
gpio_mode_t led1_mode = GPIO_OUT;

gpio_t doorSensor = TRIGGER;

uint8_t status = 0;

//uint32_t last_interrupt_time = 0;  // Time last interrupt occurs
//int door_state = -1;  // -1: invalid, init with invalid value TODO for error checking
//volatile bool busy = false;

void trigger_door_callback(void *arg) {
    
    (void) arg; /* the argument is not used */

    if (!gpio_read(doorSensor)) {
        // TO SMTH
        status = 0;
        update_status(status);
        event_post(EVENT_PRIO_HIGHEST, &eventA0);
        gpio_set(led1);
        
    }
    else {
        // TO SMTH
        status = 1;
        update_status(status);
        event_post(EVENT_PRIO_HIGHEST, &eventA0);
        gpio_clear(led1);
        
    }  
}

int initial_door_state(void) {
    gpio_init(led1, led1_mode);
    gpio_clear(led1);
    return gpio_read(doorSensor);
}

void init__door_interrupt(void) {
    init_event();
    gpio_init_int(doorSensor, GPIO_IN_PU, GPIO_BOTH, trigger_door_callback, NULL);
}

