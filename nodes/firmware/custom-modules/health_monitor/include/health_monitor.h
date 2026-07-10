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
#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include "battery_voltage_monitor.h"
#include "shock_detector.h"
#include "personalization.h"
#include "mate_lorawan.h"
#include "health_monitor_serialization.h"
#include "health_monitor_payload.h"

#define LOG_LEVEL LOG_INFO
#include "log.h"
#include "thread.h"
#include <stdlib.h>

#define LOW_BATTERY_THRESHOLD_MV 3700
#define BATTERY_UPDATE_PERIOD_SEC 5

typedef struct {
	bool battery_monitor_running;
	kernel_pid_t battery_thread_pid;
	char battery_thread_stack[THREAD_STACKSIZE_DEFAULT];
	battery_voltage_monitor_t battery_instance;
	
	
	bool shock_detector_running;
	kernel_pid_t shock_detector_thread_pid;
	char shock_detector_thread_stack[THREAD_STACKSIZE_DEFAULT];
	shock_detector_t shock_detector_instance;
} health_monitor_t;

int health_monitor_init(health_monitor_t* instance);
int health_monitor_start(health_monitor_t* instance);

#endif