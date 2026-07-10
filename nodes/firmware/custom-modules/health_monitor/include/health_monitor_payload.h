/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Payload structure for the health monitoring for SenseGate
 * @author      Maverick Widjaja <Maverick.widjaja@haw-hamburg.de>
 */
#ifndef HEALTH_MONITOR_PAYLOAD_H
#define HEALTH_MONITOR_PAYLOAD_H

#include <stdint.h>

#define HEALTH_MONITOR_PAYLOAD_ELEMENTS 2

typedef uint8_t health_monitor_header_t;
enum {
	BATTERY_CHARGING = 0,
	BATTERY_DISCHARGING,
	BATTERY_LOW,
	ACCELEROMETER
};



typedef struct {
	health_monitor_header_t header;
	int16_t body;
} health_monitor_payload_t; // total 6 elements

#endif