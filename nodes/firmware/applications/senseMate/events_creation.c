#include "include/events_creation.h"
#include <stdio.h>
#include "ztimer.h"
#include "board.h"
#include "event/timeout.h"
#include "include/sound.h"
#include "include/vibrationModule.h"
#include "include/sensemate_ui.h"
#define LOG_LEVEL   LOG_NONE
#include "log.h"
#define MIN_SIGNAL_STRENGTH -100 // Minimum signal strength for events
#define DECREMENT_RSSI 10 // decrement rssi by 10
#define RSSI_DECREMENT_TIMEOUT 5000 // in milliseconds

extern event_t event_trigger_ui_refresh;
event_queue_t events_creation_queue;
static char thread_stack[THREAD_STACKSIZE_DEFAULT];
extern sound_module_t sound_module;

void* thread_events_function(void *arg) {
    (void)arg; // Unused argument

    event_queue_init(&events_creation_queue); // Initialize the sound event queue

    while(1){
        puts("events creation thread running.");
        event_loop(&events_creation_queue);
    }
}

void init_event(void){
    thread_create(thread_stack, sizeof(thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, thread_events_function, NULL,
                  "events_creation");
}


//void event_callback (void *arg)
//{
//    (void) arg; /* the argument is not used */
//    ztimer_sleep(ZTIMER_MSEC, 500); // Wait for 0.1 second before accepting the next event
//}

void event_handlerNews(event_t *event)
{
    (void) event;   /* Not used */

    LOG_DEBUG("[events_creation]: got news\n");
    start_vibration();
    ui_data_t *ui_state = sensemate_ui_get_state();
    ui_state->lora_state = RECEIVED;
    //_update_ui();
    sound_play(&sound_module, &downlink_rx_seq);

    //downlink_reveived_sound();
    stop_vibration();
}

void event_handlerBleNews(event_t *event)
{
    (void) event;   /* Not used */

    LOG_DEBUG("[events_creation]: got ble news\n");
    //start_vibration();
    ui_data_t *ui_state = sensemate_ui_get_state();
    ui_state->ble_state = RECEIVED;
    //_update_ui();

    event_post(EVENT_PRIO_MEDIUM, &event_trigger_ui_refresh);

    sound_play(&sound_module, &tables_news_seq);
    //stop_vibration();
}

void event_handlerBleRx(event_t *event)
{
    (void) event;   /* Not used */
    ui_data_t *ui_state = sensemate_ui_get_state();
    ui_state->ble_state = RECEIVED;
    //_update_ui();
    sound_play(&sound_module, &ble_rx_seq);
}

void event_handlerBleTx(event_t *event)
{
    (void) event;   /* Not used */
    ui_data_t *ui_state = sensemate_ui_get_state();
    ui_state->ble_state = TRANSMITTED;
    //_update_ui();
}

event_t eventNews = { .handler = event_handlerNews };
event_t eventBleNews = { .handler = event_handlerBleNews };
event_t eventBleRx = { .handler = event_handlerBleRx };
event_t eventBleTx = { .handler = event_handlerBleTx };
