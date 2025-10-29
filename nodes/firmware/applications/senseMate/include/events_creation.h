#ifndef _EVENTS_CREATION_H
#define _EVENTS_CREATION_H

#include "periph/gpio.h"
#include "thread.h"
#include "event.h"
#include "event/thread.h"

/*
 * Event handler for news received through LoRaWAN
*/
void event_handlerNews(event_t *event);

/*
 * Event handler for news received through BLE
*/
void event_handlerBleNews(event_t *event);

// event for news received through LoRaWAN
extern event_t eventNews;

// event for something received through BLE
extern event_t eventBleRx;

// event for something transmitted via BLE
extern event_t eventBleTx;

// event for news received through BLE
extern event_t eventBleNews;

// event for debouncing inputs
extern event_t event_reactivate;

// eventqueue for events_creation-module
extern event_queue_t events_creation_queue;

/*
 * Initializes the event system and starts the event thread
 */
extern void init_event(void);

#endif
