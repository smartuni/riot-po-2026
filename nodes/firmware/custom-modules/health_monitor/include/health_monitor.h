#pragma once
#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include "battery_voltage_monitor.h"
#include "shock_detector.h"
#include "personalization.h"
#include "mate_lorawan.h"

#include <stdlib.h>


typedef struct {
	shock_status_t shock_status; // 1 byte
	battery_info_t battery_info; // 3 bytes
} health_monitor_payload_t; // total 4 bytes
//#define HEALTH_MONITOR_PAYLOAD_SIZE sizeof(health_monitor_payload_t)
typedef struct {
	battery_voltage_monitor_t* battery_monitor;
	shock_detector_t* shock_detector;
} health_monitor_t;

health_monitor_t* health_monitor_new(void);
int health_monitor_start(health_monitor_t* monitor);
int health_monitor_stop(health_monitor_t* monitor);

#endif