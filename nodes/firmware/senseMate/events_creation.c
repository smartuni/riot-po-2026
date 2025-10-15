#include "include/events_creation.h"
#include <stdio.h>
#include "ztimer.h"
#include "board.h"
#include "event/timeout.h"
#include "include/new_menu.h"
#include "include/soundModule.h"
#include "include/vibrationModule.h"
#include "tables.h"
#include "include/sensemate_ui.h"
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

static void _update_ui(void){
    ui_data_t data = {
        .visible_gate_cnt = tables_get_is_state_entry_count(),
        .pending_jobs_cnt = tables_get_jobs_entry_count(),
        .visible_mate_cnt = tables_get_seen_state_entry_count(),
    };

    sensemate_ui_update(&data);
}

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
        if(get_timestamp_entry(i, &timestamp_tbl_entry_buf)==TABLE_SUCCESS){
            if(timestamp_tbl_entry_buf.rssi > MIN_SIGNAL_STRENGTH){
                timestamp_tbl_entry_buf.rssi -= DECREMENT_RSSI; //decrement rssi by 10
                printf("Decremented rssi for gate %d to %d\n", timestamp_tbl_entry_buf.gateID, timestamp_tbl_entry_buf.rssi);
            }
            set_timestamp_entry(&(timestamp_entry){
                .gateID = timestamp_tbl_entry_buf.gateID,
                .timestamp = timestamp_tbl_entry_buf.timestamp,
                .rssi = timestamp_tbl_entry_buf.rssi
            });
        };
    }
    _update_ui();
    event_timeout_set(&decrement_rssi_timeout, RSSI_DECREMENT_TIMEOUT); // Set a timeout for the next decrement
}

void event_handler_reactivate(event_t *event)
{
    (void) event;   /* Not used */
    event_accepted = true; // Allow the event handler to be called again
    
}

void event_handlerNews(event_t *event)
{
    (void) event;   /* Not used */
    
    puts("got news");
    start_vibration();
    _update_ui();
    event_post(&sound_queue, &downlink_sound_event);
    //downlink_reveived_sound();
    event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
    stop_vibration();
}

void event_handlerBleNews(event_t *event)
{
    (void) event;   /* Not used */
    
    puts("got ble news");
    start_vibration();
    _update_ui();
    event_post(&sound_queue, &tables_news_sound_event);
    //ble_reveived_sound();
    event_timeout_set(&reactivate, 250); // Set a timeout to allow reactivation
    stop_vibration();
    
}

event_t eventA0 = { .handler = event_handlerA0 };
event_t eventA1 = { .handler = event_handlerA1 };
event_t eventA3 = { .handler = event_handlerA3 };
event_t eventNews = { .handler = event_handlerNews };
event_t eventBleNews = { .handler = event_handlerBleNews };
event_t event_reactivate = { .handler = event_handler_reactivate };
