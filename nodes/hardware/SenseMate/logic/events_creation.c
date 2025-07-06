#include "events_creation.h"
#include <stdio.h>
//#include "menu.h"
#include "ztimer.h"
#include "board.h"
#include "event/timeout.h"
#include "new_menu.h"
#include "soundModule.h"



bool event_accepted = true;
event_timeout_t reactivate;

void init_event(void){
    event_timeout_ztimer_init(&reactivate, ZTIMER_MSEC, EVENT_PRIO_HIGHEST, &event_reactivate);
}


void event_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    ztimer_sleep(ZTIMER_MSEC, 500); // Wait for 0.1 second before accepting the next event
    event_accepted = true; // Allow the event handler to be called again
}

void event_handler_reactivate(event_t *event)
{
    (void) event;   /* Not used */
    event_accepted = true; // Allow the event handler to be called again
    
}


void event_handlerA0(event_t *event)
{
    (void) event;   /* Not used */
    if(event_accepted){
        event_accepted = false; // Prevent further calls until reset
        menu_input(DOWN);
        //set_current_meustate(DOWN);
        puts("A0 event handler was called.");

        event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
        //refresh_display();
    }else{
        puts("event A0 ignored");
    }
    
}

void event_handlerA1(event_t *event)
{
    (void) event;   /* Not used */
    if(event_accepted){
        event_accepted = false; // Prevent further calls until reset
        //set_current_meustate(SELECT);
        menu_input(SELECT);
        puts("A1 event handler was called.");
        event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
        ///refresh_display();
    }else{
        puts("event A1 ignored");
    }
}

void event_handlerA3(event_t *event)
{
    (void) event;   /* Not used */
    if(event_accepted){
        event_accepted = false; // Prevent further calls until reset
        //set_current_meustate(UP);
        menu_input(UP);
        puts("A3 event handler was called.");
        event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
        //refresh_display();
    }else{
        puts("event A3 ignored");
    }
    
}

void event_handlerNews(event_t *event)
{
    (void) event;   /* Not used */
    if(event_accepted){
        puts("got news");
        update_menu_from_tables();
        downlink_reveived_sound();
        //ble_reveived_sound();
        event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
    }else{
        puts("event news ignored");
    }
}

event_t eventA0 = { .handler = event_handlerA0 };
event_t eventA1 = { .handler = event_handlerA1 };
event_t eventA3 = { .handler = event_handlerA3 };
event_t eventNews = { .handler = event_handlerNews };
event_t event_reactivate = { .handler = event_handler_reactivate };