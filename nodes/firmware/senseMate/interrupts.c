#include "include/interrupts.h"
#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "include/displayDemo.h"
#include "include/events_creation.h"
#include "include/soundModule.h"

#define TRIGGER4 GPIO_PIN(1, 2);

// for first implementation when interrupt is registered, led is on


gpio_t led0 = GPIO_PIN(1, 10);
gpio_mode_t led0_mode = GPIO_OUT;
gpio_t trigger4 = TRIGGER4;

void trigger4_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(trigger4)) {
        // TO SMTH
        printf("Trigger 4 activated\n");
        event_post(&events_creation_queue, &eventNews);
        event_post(&sound_queue, &ble_received_sound_event);
    }
    else {
        // TO SMTH
        gpio_clear(led0);
    }
}

void init_interrupt(void){
    gpio_init(led0, led0_mode);
    gpio_clear(led0);
    
    gpio_init_int(trigger4, GPIO_IN_PU, GPIO_FALLING, trigger4_callback, NULL);
}
