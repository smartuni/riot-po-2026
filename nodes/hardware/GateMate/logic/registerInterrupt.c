#include "header/registerInterrupt.h"

#define TRIGGER GPIO_PIN(1, 2);

// for first implementation when interrupt is registered, led is on


gpio_t led0 = GPIO_PIN(1, 10);
gpio_mode_t led0_mode = GPIO_OUT;
gpio_t trigger = TRIGGER;

void trigger_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(trigger)) {
        // TO SMTH
        gpio_set(led0);
    }
    else {
        // TO SMTH
        gpio_clear(led0);
    }
}


void init_interrupt(void){
    gpio_init(led0, led0_mode);
    gpio_clear(led0);
    
    gpio_init_int(trigger, GPIO_IN_PU, GPIO_BOTH, trigger_callback, NULL);
}