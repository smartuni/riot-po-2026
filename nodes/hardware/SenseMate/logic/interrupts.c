#include "interrupts.h"
#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "displayDemo.h"
#include "menu.h"
#include "events_creation.h"
#include "soundModule.h"

#define TRIGGER0 GPIO_PIN(0, 4);
#define TRIGGER1 GPIO_PIN(0, 5);
#define TRIGGER3 GPIO_PIN(0, 28);
#define TRIGGER4 GPIO_PIN(1, 2);

// for first implementation when interrupt is registered, led is on


gpio_t led0 = GPIO_PIN(1, 10);
gpio_mode_t led0_mode = GPIO_OUT;
gpio_t trigger0 = TRIGGER0;
gpio_t trigger1 = TRIGGER1;
gpio_t trigger3 = TRIGGER3;
gpio_t trigger4 = TRIGGER4;

void trigger0_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(trigger0)) {
        // TO SMTH
        //gpio_set(led0);
        printf("Trigger 0 activated\n");
        event_post(&events_creation_queue, &eventA0);
    }
    else {
        // TO SMTH
        gpio_clear(led0);
    }
}

void trigger1_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(trigger1)) {
        // TO SMTH
        //gpio_set(led0);
        printf("Trigger 1 activated\n");
        event_post(&events_creation_queue, &eventA1);
    }
    else {
        // TO SMTH
        gpio_clear(led0);
    }
}

void trigger3_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    if (!gpio_read(trigger3)) {
        // TO SMTH
        //gpio_set(led0);
        printf("Trigger 3 activated\n");
        event_post(&events_creation_queue, &eventA3);
    }
    else {
        // TO SMTH
        gpio_clear(led0);
    }
}

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
    
    gpio_init_int(trigger0, GPIO_IN_PU, GPIO_FALLING, trigger0_callback, NULL);
    gpio_init_int(trigger1, GPIO_IN_PU, GPIO_FALLING, trigger1_callback, NULL);
    gpio_init_int(trigger3, GPIO_IN_PU, GPIO_FALLING, trigger3_callback, NULL);
    gpio_init_int(trigger4, GPIO_IN_PU, GPIO_FALLING, trigger4_callback, NULL);
}