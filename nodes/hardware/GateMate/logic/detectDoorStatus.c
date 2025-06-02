#include "header/detectDoorStatus.h"

#define DEBOUNCE_TIME 200
// #define TRIGGER GPIO_PIN(0, 3) // PIN A5
#define TRIGGER GPIO_PIN(1, 2) // Test Button

gpio_t doorSensor = TRIGGER;

uint32_t last_interrupt_time = 0;  // Time last interrupt occurs
int door_state = -1;  // -1: invalid, init with invalid value TODO for error checking
volatile bool busy = false;

void trigger_door_callback(void *arg) {
    (void) arg;  // no use
    

    uint32_t current_time = ztimer_now(ZTIMER_MSEC);  

    door_state = gpio_read(doorSensor);

    // Ignore change if DEBOUNCE_TME is bigger
    if (busy){
        if ((current_time - last_interrupt_time) < DEBOUNCE_TIME) {
            puts("Ignore Interrupt: Time since last interrupt below threshold");
            return;
        } else {
            busy = false;
        }
    }

    last_interrupt_time = current_time;

    if (door_state == 0) {
        puts("Door has been closed");
        // TODO: Write to Table
    
    } else if (door_state == 1){
        puts("Door has been opened");
        // TODO: Write to Table
    } else {
        door_state = -1;
        puts("Illegal state");
        // ERROR
    }
}

int initial_door_state(void) {
    return gpio_read(doorSensor);
}

void init__door_interrupt(void) {
    gpio_init_int(doorSensor, GPIO_IN_PU, GPIO_BOTH, trigger_door_callback, NULL);
}