#ifndef REGINTERRUPT_H
#define REGINTERRUPT_H

#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"


/**
* initializes interrupt for the sensor input
*/
void init_interrupt(void);


#endif