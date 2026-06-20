#pragma once
#include <stdint.h>

#define HEALTH_MONITOR_PAYLOAD_SIZE 4

typedef uint8_t shock_status_t;
enum {
	NO_SHOCK = 0,
	SHOCK_DETECTED
};

typedef uint8_t battery_status_t;
enum {
	CHARGING = 0,
	DISCHARGING,
	DISCHARGING_LOW_BATTERY
};

typedef struct{
	battery_status_t battery_status;
	int16_t voltage_mv; 
} battery_info_t; //1+2 bytes


typedef struct {
	shock_status_t shock_status; // 1 byte
	battery_info_t battery_info; // 3 bytes
} health_monitor_payload_t; // total 4 bytes