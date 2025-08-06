#ifndef _EVENTS_CREATION_H
#define _EVENTS_CREATION_H

#include "periph/gpio.h"
#include "thread.h"
#include "event.h"
#include "event/thread.h"

/*
 * Event handler for inputs on the FeatherSense Pin A0
 * currently used for the menu input DOWN
 * The through the events inputs are debounced.
 */
void event_handlerA0(event_t *event);

/*
 * Event handler for inputs on the FeatherSense Pin A1
 * currently used for the menu input SELECT
 * The through the events inputs are debounced.
 */
void event_handlerA1(event_t *event);

/*
 * Event handler for inputs on the FeatherSense Pin A3
 * currently used for the menu input UP
 * The through the events inputs are debounced.
 */
void event_handlerA3(event_t *event);

/*
 * Event handler for news received through LoRaWAN
*/
void event_handlerNews(event_t *event);

/*
 * Event handler for news received through BLE
*/
void event_handlerBleNews(event_t *event);

// event for event at Pin A0
extern event_t eventA0;

// event for event at Pin A1
extern event_t eventA1;

// event for event at Pin A3
extern event_t eventA3;

// event for news received through LoRaWAN
extern event_t eventNews;

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