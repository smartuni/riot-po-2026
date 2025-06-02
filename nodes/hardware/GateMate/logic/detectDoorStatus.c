#include "header/detectDoorStatus.h"

#define TRIGGER GPIO_PIN(0, 3); // PIN A5

gpio_t doorSensor = TRIGGER;

void trigger_door_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(doorSensor)) {
        // TODO  WRITE STATE TO TABLE
        puts("door has been closed");
    }
    else {
        // TODO  WRITE STATE TO TABLE
        puts("door has been opened");
    }
}

int initial_door_state(void){
    return gpio_read(doorSensor);
}

void init__door_interrupt(void){   
    gpio_init_int(doorSensor, GPIO_IN_PU, GPIO_BOTH, trigger_door_callback, NULL);
}