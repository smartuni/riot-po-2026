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
	uint32_t raw_voltage = battery_voltage_sample(AIN7_BAT);
	uint32_t voltage_mv = battery_voltage_sample2adc_voltage(raw_voltage);
	return (int)voltage_mv;
}

int battery_voltage_monitor_init(battery_voltage_monitor_t* instance) {
	gpio_t adc_pin = GPIO_PIN(0, 14);
	int res = voltage_adc_setup(adc_pin, AIN7_BAT);
	if (res != 0) {
		LOG_DEBUG("[battery_voltage_monitor.c:%d] Error setting up voltage adc\n", __LINE__);
		return -1;
	}
	instance->prev_voltage_mv = -1;

	return 0;
}

static voltage_trend analyze_voltage_trend(const int prev_voltage_mv, const int current_voltage_mv) {
	if (current_voltage_mv > prev_voltage_mv) {
		return VOLTAGE_TREND_INCREASING;
	} else if (current_voltage_mv < prev_voltage_mv) {
		return VOLTAGE_TREND_DECREASING;
	} else if (current_voltage_mv == prev_voltage_mv) {
		return VOLTAGE_TREND_STABLE;
	}
	return VOLTAGE_TREND_UNKNOWN;
}

void battery_voltage_monitor_fetch_info(battery_voltage_monitor_t* instance, battery_info_t* info) {
	assert(instance != NULL);
	assert(info != NULL);
	info->voltage_mv = get_battery_voltage();
	if (instance->prev_voltage_mv == -1) {
		instance->prev_voltage_mv = info->voltage_mv;
	}
	LOG_DEBUG("[battery_voltage_monitor.c:%d] Current battery voltage: %d mV\n", __LINE__, info->voltage_mv);
	voltage_trend trend = analyze_voltage_trend(instance->prev_voltage_mv, info->voltage_mv);
	switch (trend) {
		case VOLTAGE_TREND_INCREASING: {
			LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is increasing\n", __LINE__);
			info->battery_status = BATTERY_STATE_CHARGING;
			break;
		}
		case VOLTAGE_TREND_DECREASING: {
			LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is decreasing\n", __LINE__);
			info->battery_status = BATTERY_STATE_DISCHARGING;
			break;
		}
		case VOLTAGE_TREND_STABLE: {
			LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery voltage is stable\n", __LINE__);
			info->battery_status = instance->last_battery_status;
			break;
		}
		default: {
			LOG_DEBUG("[battery_voltage_monitor.c:%d] Battery status is unknown\n", __LINE__);
			info->battery_status = BATTERY_STATE_UNKNOWN;
			break;
		}
	}
	instance->last_battery_status = info->battery_status;
	instance->prev_voltage_mv = info->voltage_mv;
}

