#ifndef DETECTDOORSTATUS_H
#define DETECTDOORSTATUS_H

#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "event_creation.h"


void init__door_interrupt(void);
int initial_door_state(void);



#endif