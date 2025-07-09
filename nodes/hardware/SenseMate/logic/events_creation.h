#ifndef _EVENTS_CREATION_H
#define _EVENTS_CREATION_H

#include "periph/gpio.h"
#include "thread.h"
#include "event.h"
#include "event/thread.h"

void event_handlerA0(event_t *event);
void event_handlerA1(event_t *event);
void event_handlerA3(event_t *event);
void event_handlerNews(event_t *event);
void event_handlerBleNews(event_t *event);

extern event_t eventA0;
extern event_t eventA1;
extern event_t eventA3;
extern event_t eventNews;
extern event_t eventBleNews;
extern event_t event_reactivate;

extern void init_event(void);

#endif