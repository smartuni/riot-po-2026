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

void event_handlerA0(event_t *event);

extern event_t eventA0;
extern event_t event_reactivate;

extern void init_event(void);

#endif