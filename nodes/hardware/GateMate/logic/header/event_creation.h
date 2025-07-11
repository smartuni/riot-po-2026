#ifndef _EVENT_CREATION_H
#define _EVENT_CREATION_H

#include "periph/gpio.h"
#include "thread.h"
#include "event.h"
#include "event/thread.h"

#include <stdio.h>
#include "ztimer.h"
#include "board.h"
#include "event/timeout.h"
#include "tables.h" 
#include "mate_lorawan.h"

#define GATE_ID DEVICE_ID
#define DEBOUNCE_TIME 5000

extern event_t eventA0;
extern event_t event_reactivate;

/**
* function that handles incoming events, that are send when an interrupt occurs. 
* Futher events will be ignored while a timed runoff.
*/
void event_handlerA0(event_t *event);

/**
* enables the event
*/
extern void init_event(void);

/**
* sets current status of sensor. needed to set internal table after change
*/
void update_status(uint8_t newStatus);



#endif