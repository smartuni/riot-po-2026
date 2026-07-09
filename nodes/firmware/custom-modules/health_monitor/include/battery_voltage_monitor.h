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
#pragma once
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

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t voltage_trend;
enum {
	VOLTAGE_TREND_UNKNOWN = 0,
	VOLTAGE_TREND_INCREASING,
	VOLTAGE_TREND_DECREASING,
	VOLTAGE_TREND_STABLE
	
};

typedef uint8_t battery_status_t;
enum {
	BATTERY_STATE_UNKNOWN = 0,
	BATTERY_STATE_CHARGING,
	BATTERY_STATE_DISCHARGING
};

typedef struct {
	battery_status_t battery_status;
	int16_t voltage_mv;
} battery_info_t; //1+2 bytes

typedef struct {
	//Structure for memory alignment and to avoid padding
	//1. classes, structs, STL containers, arrays
	// char thread_stack[THREAD_STACKSIZE_DEFAULT];
	// kernel_pid_t thread_pid;
	//2. pointers
	// void (*callback)(void);
	//3. primitive types (int, double)
	//int threshold_mv;
	int prev_voltage_mv;
	battery_status_t last_battery_status;
	//4. bool and char
} battery_voltage_monitor_t;


int battery_voltage_monitor_init(battery_voltage_monitor_t* instance);

battery_info_t battery_voltage_monitor_fetch_info(battery_voltage_monitor_t* instance);

/**
 * @brief Deletes the battery voltage monitor
 * @param monitor Pointer to the battery voltage monitor
 * @return always return 0
 */
int battery_voltage_monitor_delete(battery_voltage_monitor_t* instance);

#endif