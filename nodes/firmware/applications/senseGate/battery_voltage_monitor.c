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
#include "battery_voltage_monitor.h"

battery_voltage_monitor_t* battery_voltage_monitor_new(int threshold_mv) {
	battery_voltage_monitor_t* monitor = (battery_voltage_monitor_t*)malloc(sizeof(battery_voltage_monitor_t));
	if (monitor == NULL) {
		LOG_BATTERY_VOLTAGE("Failed to allocate memory for battery voltage monitor");
		return NULL;
	}
	monitor->threshold_mv = threshold_mv;
	monitor->prev_voltage_mv = -1;
	monitor->running = false;
	//monitor->battery_status = UNKNOWN;
	monitor->last_voltage_trend = UNKNOWN;
	return monitor;
}

static int get_battery_voltage(void) {
	return 3700; //TODO replace this with actual batt voltage reading from Colin's code
}

static enum voltage_trend analyze_voltage_trend(const int prev_voltage_mv, const int current_voltage_mv, const int threshold_mv) {
	if (current_voltage_mv > prev_voltage_mv) {
		return INCREASING;
	} else if (current_voltage_mv < prev_voltage_mv) {
		return DECREASING;
	} else if (current_voltage_mv == prev_voltage_mv) {
		return STABLE;
	}
	return UNKNOWN;
}

static void* battery_voltage_thread(void* monitor_void) {
	battery_voltage_monitor_t* monitor = (battery_voltage_monitor_t*)monitor_void;
	int voltage_mv = get_battery_voltage();
	if (monitor->prev_voltage_mv == -1) {
		monitor->prev_voltage_mv = voltage_mv;
	}
	while (monitor->running) {
		int voltage_mv = get_battery_voltage();
		LOG_BATTERY_VOLTAGE("Current battery voltage: %d mV\n", voltage_mv);
		if (voltage_mv < monitor->threshold_mv) {
			LOG_BATTERY_VOLTAGE("Battery voltage is below threshold! (%d mV < %d mV)\n", voltage_mv, monitor->threshold_mv);
			// TODO report low battery to LoRaWAN
		} else {
            LOG_BATTERY_VOLTAGE("Battery voltage is nominal: %d mV\n", voltage_mv);
			enum voltage_trend trend = analyze_voltage_trend(monitor->prev_voltage_mv, voltage_mv, monitor->threshold_mv);
			switch (trend) {
			case INCREASING:
				LOG_BATTERY_VOLTAGE("Battery voltage is increasing\n");
				if (monitor->last_voltage_trend != INCREASING) {
					// TODO report CHARGING to LoRaWAN
					monitor->last_voltage_trend = INCREASING;
				}
				break;
			case DECREASING:
				LOG_BATTERY_VOLTAGE("Battery voltage is decreasing\n");
				if (monitor->last_voltage_trend != DECREASING) {
					// TODO report DISCHARGING to LoRaWAN
					monitor->last_voltage_trend = DECREASING;
				}
				break;
			case STABLE:
				LOG_BATTERY_VOLTAGE("Battery voltage is stable\n");
				break;
			default:
				LOG_BATTERY_VOLTAGE("Battery status is unknown\n");
				break;
			}
		}
		ztimer_sleep(ZTIMER_MSEC, 5000); // Check every 5 seconds
		monitor->prev_voltage_mv = voltage_mv;
	}
	return NULL;
}

int battery_voltage_monitor_start(battery_voltage_monitor_t* monitor) {
	if (monitor == NULL) {
		LOG_BATTERY_VOLTAGE("Invalid battery voltage monitor");
		return -1;
	}
	monitor->running = true;
	monitor->thread_pid = thread_create(monitor->thread_stack, sizeof(monitor->thread_stack),
										THREAD_PRIORITY_MAIN - 1, 0,
										battery_voltage_thread, monitor, "battery_voltage_thread");
	return 0;
}

int battery_voltage_monitor_delete(battery_voltage_monitor_t* monitor) {
	if (monitor == NULL) {
		LOG_BATTERY_VOLTAGE("Invalid battery voltage monitor");
		return -1;
	}
	monitor->running = false;
	ztimer_sleep(ZTIMER_MSEC, 100); // Give the thread some time to exit
	free(monitor);
	return 0;
}