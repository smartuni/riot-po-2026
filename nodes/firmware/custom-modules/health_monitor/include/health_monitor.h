#pragma once
#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include "battery_voltage_monitor.h"
#include "personalization.h"
#include "mate_lorawan.h"

#include <stdlib.h>


typedef struct {
	battery_status battery_status;
	int voltage_mv;
	shock_status shock_status;
} health_monitor_payload_t;

typedef struct {
	int placeholder;
} health_monitor_t;

health_monitor_t* health_monitor_init(void);
int health_monitor_start(health_monitor_t* monitor);
int health_monitor_stop(health_monitor_t* monitor);

#endif