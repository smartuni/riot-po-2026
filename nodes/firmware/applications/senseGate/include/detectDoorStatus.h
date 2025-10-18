#ifndef DETECTDOORSTATUS_H
#define DETECTDOORSTATUS_H

#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "periph/gpio.h"
#include "event_creation.h"



/**
* intitializes interrupts. 
*/
void init__door_interrupt(void);


/**
* returns the actual state of the sensor.
* is being called during initialisation.
* @return 0 or 1 for initial status of the sensor
*/
uint8_t initial_door_state(void);



#endif