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

#define GATE_ID 1
#define DEBOUNCE_TIME 10000



// #include "mate_lorawan.h"

void event_handlerA0(event_t *event);



extern event_t eventA0;
extern event_t event_reactivate;

extern void init_event(void);
void update_status(uint8_t newStatus);

void setTimestamp(int newTimestamp);
int getTimestamp(void);

#endif