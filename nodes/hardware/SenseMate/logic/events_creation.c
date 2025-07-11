#include "events_creation.h"
#include <stdio.h>
//#include "menu.h"
#include "ztimer.h"
#include "board.h"
#include "event/timeout.h"
#include "new_menu.h"
#include "soundModule.h"
#include "vibrationModule.h"
#include "tables.h"
#define MIN_SIGNAL_STRENGTH -100 // Minimum signal strength for events
#define DECREMENT_RSSI 10 // decrement rssi by 10
#define RSSI_DECREMENT_TIMEOUT 5000 // in milliseconds


bool event_accepted = true;
event_timeout_t reactivate;
event_timeout_t decrement_rssi_timeout;
event_queue_t events_creation_queue;
static char thread_stack[THREAD_STACKSIZE_DEFAULT];
static timestamp_entry timestamp_tbl_entry_buf;
void event_handler_decrement_rssi_timeout(event_t *event);
event_t event_rssi_timeout = { .handler = event_handler_decrement_rssi_timeout };

void* thread_events_function(void *arg) {
    (void)arg; // Unused argument
    
    event_queue_init(&events_creation_queue); // Initialize the sound event queue

    while(1){
        puts("events creation thread running.");
        event_loop(&events_creation_queue);
    }
}

void init_event(void){
    event_timeout_ztimer_init(&reactivate, ZTIMER_MSEC, &events_creation_queue, &event_reactivate);
    
    event_timeout_ztimer_init(&decrement_rssi_timeout, ZTIMER_MSEC, &events_creation_queue, &event_rssi_timeout);
    thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, thread_events_function, NULL, "events_creation_thread");
    
    event_timeout_set(&decrement_rssi_timeout, RSSI_DECREMENT_TIMEOUT); // Set a timeout for the next decrement
}


void event_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    ztimer_sleep(ZTIMER_MSEC, 500); // Wait for 0.1 second before accepting the next event
    event_accepted = true; // Allow the event handler to be called again
}

void event_handler_decrement_rssi_timeout(event_t *event)
{
    (void) event;   /* Not used */
    event_accepted = true; // Allow the event handler to be called again
    for(int i = 0; i< MAX_GATE_COUNT; i++){
        if(get_timestamp_entry(i, &timestamp_tbl_entry_buf)!=TABLE_ERROR_NOT_FOUND){
            if(timestamp_tbl_entry_buf.rssi > MIN_SIGNAL_STRENGTH){
                timestamp_tbl_entry_buf.rssi -= DECREMENT_RSSI; //decrement rssi by 10
            }
            set_timestamp_entry(&(timestamp_entry){
                .gateID = timestamp_tbl_entry_buf.gateID,
                .timestamp = timestamp_tbl_entry_buf.timestamp,
                .rssi = timestamp_tbl_entry_buf.rssi
            });
        };
    }
    update_menu_from_tables();
    update_menu_display();
    event_timeout_set(&decrement_rssi_timeout, RSSI_DECREMENT_TIMEOUT); // Set a timeout for the next decrement
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
    
    puts("got news");
    start_vibration();
    update_menu_from_tables();
    event_post(&sound_queue, &downlink_sound_event);
    //downlink_reveived_sound();
    event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
    update_menu_display();
    stop_vibration();
}

void event_handlerBleNews(event_t *event)
{
    (void) event;   /* Not used */
    
    puts("got ble news");
    start_vibration();
    update_menu_from_tables();
    event_post(&sound_queue, &tables_news_sound_event);
    //ble_reveived_sound();
    event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
    update_menu_display();
    stop_vibration();
    
}

event_t eventA0 = { .handler = event_handlerA0 };
event_t eventA1 = { .handler = event_handlerA1 };
event_t eventA3 = { .handler = event_handlerA3 };
event_t eventNews = { .handler = event_handlerNews };
event_t eventBleNews = { .handler = event_handlerBleNews };
event_t event_reactivate = { .handler = event_handler_reactivate };
