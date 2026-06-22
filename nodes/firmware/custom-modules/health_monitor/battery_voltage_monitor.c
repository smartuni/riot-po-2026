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

static int voltage_adc_setup(gpio_t pwr_pin, adc_t line) {
	int res = adc_init(line);
	if (res != 0) {
		return -1;
	}
	res = gpio_init(pwr_pin, GPIO_OUT);
	if (res != 0) {
		return -3;
	}
	gpio_clear(pwr_pin);
	return 0;
}

static uint32_t battery_voltage_sample(adc_t line) {
	int32_t sample = adc_sample(line, ADC_RES_12BIT);
	return sample;
}

static uint32_t battery_voltage_sample2adc_voltage(uint32_t sample) {
	uint32_t max_val = ((1 << 12) - 1);
	double voltage = (double)sample * 3300 / max_val / (51.0 / 151.0);
	return (uint32_t)voltage; //3300 is voltage, 51/151 is the ratio of resistors in voltage divider of seeed xiao n5f52840 battery charge circuit
}

static int get_battery_voltage(void) {
	//TODO adjust based on the voltage divider
	uint32_t raw_voltage = battery_voltage_sample(AIN7_BAT);
	uint32_t voltage_mv = battery_voltage_sample2adc_voltage(raw_voltage);

	LOG_DEBUG("[battery_voltage_monitor.c:%d] Hey!Raw data is %d Voltage is: %d\n", __LINE__, (int)raw_voltage, (int)voltage_mv);
	return 3700;
}

battery_voltage_monitor_t* battery_voltage_monitor_new(int threshold_mv) {
	gpio_t adc_pin = GPIO_PIN(0, 14);
	int res = voltage_adc_setup(adc_pin, AIN7_BAT);
	if (res != 0) {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Error setting up voltage adc\n", __LINE__);
		return NULL;
	}
	battery_voltage_monitor_t* monitor = (battery_voltage_monitor_t*)malloc(sizeof(battery_voltage_monitor_t));
	if (monitor == NULL) {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Failed to allocate memory for battery voltage monitor", __LINE__);
		return NULL;
	}
	monitor->threshold_mv = threshold_mv;
	monitor->prev_voltage_mv = -1;
	//monitor->running = false;

	return monitor;
}

static voltage_trend analyze_voltage_trend(const int prev_voltage_mv, const int current_voltage_mv) {
	if (current_voltage_mv > prev_voltage_mv) {
		return INCREASING;
	} else if (current_voltage_mv < prev_voltage_mv) {
		return DECREASING;
	} else if (current_voltage_mv == prev_voltage_mv) {
		return STABLE;
	}
	return UNKNOWN;
}

void battery_voltage_monitor_get_info(battery_voltage_monitor_t* monitor, battery_info_t* info) {
	int voltage_mv = get_battery_voltage();
	if (monitor->prev_voltage_mv == -1) {
		monitor->prev_voltage_mv = voltage_mv;
	}
	LOG_DEBUG("[battery_voltage_monitor.c:%d] Current battery voltage: %d mV\n", __LINE__, voltage_mv);
	if (voltage_mv < monitor->threshold_mv) {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is below threshold! (%d mV < %d mV)\n", __LINE__, voltage_mv, monitor->threshold_mv);
		info->battery_status = DISCHARGING_LOW_BATTERY;
		info->voltage_mv = voltage_mv;
	} else {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is nominal: %d mV\n", __LINE__, voltage_mv);
		voltage_trend trend = analyze_voltage_trend(monitor->prev_voltage_mv, voltage_mv);
		switch (trend) {
			case INCREASING: {
				LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is increasing\n", __LINE__);
				info->battery_status = CHARGING;
				info->voltage_mv = voltage_mv;
				break;
			}
			case DECREASING: {
				LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is decreasing\n", __LINE__);
				info->battery_status = DISCHARGING;
				info->voltage_mv = voltage_mv;
				break;
			}
			case STABLE: {
				LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is stable\n", __LINE__);
				info->battery_status = STABLE;
				info->voltage_mv = voltage_mv;
				break;
			}
			default: {
				LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery status is unknown\n", __LINE__);
				break;
			}
		}
	}
	monitor->prev_voltage_mv = voltage_mv;
}

int battery_voltage_monitor_delete(battery_voltage_monitor_t* monitor) {
	if (monitor == NULL) {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Invalid battery voltage monitor", __LINE__);
		return -1;
	}
	free(monitor);
	return 0;
}
