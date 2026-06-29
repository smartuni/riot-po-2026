/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Health Monitoring for SenseGate
 * @author      Maverick Widjaja <Maverick.widjaja@haw-hamburg.de>
 */
#pragma once
#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include "battery_voltage_monitor.h"
#include "shock_detector.h"
#include "personalization.h"
#include "mate_lorawan.h"
#include "health_monitor_serialization.h"



#define LOG_LEVEL LOG_DEBUG
#include "log.h"
#include "thread.h"
#include <stdlib.h>

//#define HEALTH_MONITOR_PAYLOAD_SIZE sizeof(health_monitor_payload_t)
typedef struct {
	kernel_pid_t battery_thread_pid;
	char battery_thread_stack[THREAD_STACKSIZE_DEFAULT];
	battery_voltage_monitor_t* battery_monitor;
	int low_battery_threshold_mv;
	bool is_low_battery;
	int battery_update_period_sec;

	shock_detector_t* shock_detector;
	int update_period_sec;
	volatile bool running;
} health_monitor_t;

health_monitor_t* health_monitor_new(void);
int health_monitor_start(health_monitor_t* monitor);
int health_monitor_delete(health_monitor_t* monitor);

#endif