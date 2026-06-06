/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Battery voltage monitor for senseGate
 * @author      Maverick Widjaja <Maverick.widjaja@haw-hamburg.de>
 */

#ifndef BATTERY_VOLTAGE_MONITOR_H
#define BATTERY_VOLTAGE_MONITOR_H

#include "thread.h"
#include "board.h"
#include "phydat.h"
#include "saul_reg.h"
#include "ztimer.h"

#define LOG_LEVEL   LOG_DEBUG
#include "log.h"
#define LOG_BATTERY_VOLTAGE(...) LOG_DEBUG("[battery_voltage]: " __VA_ARGS__)


#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#define TEMPERATURE_THRESHOLD 2400 /* factor of 10^-2 */

// Restructures for memory alignment and to avoid padding
typedef struct {
	kernel_pid_t accel_thread_pid;
	saul_reg_t* accel_sensor;
	void (*callback)(void);
	float threshold;
	volatile bool running;
	char accel_thread_stack[THREAD_STACKSIZE_DEFAULT];
} battery_voltage_monitor_t;


/**
 * @brief Creates a new battery voltage monitor to the heap
 * @return Pointer to the new battery voltage monitor, or NULL if memory allocation failed
 */
battery_voltage_monitor_t* battery_voltage_monitor_new(float threshold);



/**
 * @brief Deletes the battery voltage monitor
 * @param monitor Pointer to the battery voltage monitor
 * @return always return 0
 */
int battery_voltage_monitor_delete(battery_voltage_monitor_t* monitor);

#endif