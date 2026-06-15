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
 * @author      Colin Johnson <colinedward.johnson@haw-hamburg.de>
 */

#ifndef BATTERY_VOLTAGE_MONITOR_H
#define BATTERY_VOLTAGE_MONITOR_H

#include "thread.h"
#include "board.h"
#include "phydat.h"
#include "saul_reg.h"
#include "ztimer.h"

#include "periph/gpio.h"
#include "periph/adc.h"

#define AIN7_BAT 7

#define LOG_LEVEL   LOG_DEBUG
#include "log.h"
#define LOG_BATTERY_VOLTAGE(...) LOG_DEBUG("[battery_voltage]: " __VA_ARGS__)


#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef uint8_t voltage_trend;
enum {
	INCREASING = 0,
	DECREASING,
	STABLE,
	UNKNOWN
};

typedef uint8_t battery_status_t;
enum {
	CHARGING = 0,
	DISCHARGING,
	DISCHARGING_LOW_BATTERY
};

typedef uint8_t shock_status_t;
enum {
	NO_SHOCK = 0,
	SHOCK_DETECTED
};

typedef struct{
	battery_status_t battery_status;
	int voltage_mv;
} battery_info_t;



typedef struct {
	//Structure for memory alignment and to avoid padding
	//1. classes, structs, STL containers, arrays
	char thread_stack[THREAD_STACKSIZE_DEFAULT];
	kernel_pid_t thread_pid;
	//2. pointers
	void (*callback)(void);
	//3. primitive types (int, double)
	int threshold_mv;
	int prev_voltage_mv;
	//enum battery_status battery_status;
	//enum voltage_trend last_voltage_trend;
	//4. bool and char
	volatile bool running;
} battery_voltage_monitor_t;



/**
 * @brief Creates a new battery voltage monitor to the heap
 * @param threshold_mv Voltage threshold in millivolts to recognize the battery as low
 * @return Pointer to the new battery voltage monitor, or NULL if memory allocation failed
 */
battery_voltage_monitor_t* battery_voltage_monitor_new(int threshold_mv);

/**
 * @brief Starts the battery voltage monitoring
 * @param monitor Pointer to the battery voltage monitor
 * @return 0 on success, -1 on failure
 */
int battery_voltage_monitor_start(battery_voltage_monitor_t* monitor);

/**
 * @brief Deletes the battery voltage monitor
 * @param monitor Pointer to the battery voltage monitor
 * @return always return 0
 */
int battery_voltage_monitor_delete(battery_voltage_monitor_t* monitor);

#endif