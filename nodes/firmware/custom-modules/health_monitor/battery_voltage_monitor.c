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
	double voltage = (double)sample * 3300 / max_val / (51.0/151.0);
	return (uint32_t)voltage;	//3300 is voltage, 51/151 is the ratio of resistors in voltage divider of seeed xiao n5f52840 battery charge circuit
}

static int get_battery_voltage(void) {
	//TODO adjust based on the voltage divider
	uint32_t raw_voltage = battery_voltage_sample(AIN7_BAT);
	uint32_t voltage_mv = battery_voltage_sample2adc_voltage(raw_voltage);

	printf("Hey!Raw data is %d Voltage is: %d\n", (int)raw_voltage, (int)voltage_mv);
	return 3700;
}

battery_voltage_monitor_t* battery_voltage_monitor_new(int threshold_mv) {
	gpio_t adc_pin = GPIO_PIN(0, 14);
	int res = voltage_adc_setup(adc_pin, AIN7_BAT);
	if (res != 0) {
		LOG_BATTERY_VOLTAGE("Error setting up voltage adc\n");
		return NULL;
	}
	battery_voltage_monitor_t* monitor = (battery_voltage_monitor_t*)malloc(sizeof(battery_voltage_monitor_t));
	if (monitor == NULL) {
		LOG_BATTERY_VOLTAGE("Failed to allocate memory for battery voltage monitor");
		return NULL;
	}
	monitor->threshold_mv = threshold_mv;
	monitor->prev_voltage_mv = -1;
	monitor->running = false;

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

static void publish_payload(battery_status_payload_t* payload) {
	// TODO report low battery to LoRaWAN
	printf("Publishing battery info: status=%d, voltage=%d mV\n", payload->status, payload->voltage_mv);
	return;
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
			battery_status_payload_t battery_info = {
				.status = DISCHARGING_LOW_BATTERY,
				.voltage_mv = voltage_mv
			};
			publish_payload(&battery_info);
		} else {
			LOG_BATTERY_VOLTAGE("Battery voltage is nominal: %d mV\n", voltage_mv);
			voltage_trend trend = analyze_voltage_trend(monitor->prev_voltage_mv, voltage_mv);
			switch (trend) {
				case INCREASING: {
					LOG_BATTERY_VOLTAGE("Battery voltage is increasing\n");
					battery_status_payload_t battery_info = {
						.status = CHARGING,
						.voltage_mv = voltage_mv
					};
					publish_payload(&battery_info);
					break;
				}
				case DECREASING: {
					LOG_BATTERY_VOLTAGE("Battery voltage is decreasing\n");
					battery_status_payload_t battery_info = {
						.status = DISCHARGING,
						.voltage_mv = voltage_mv
					};
					publish_payload(&battery_info);
					break;
				}
				case STABLE: {
					LOG_BATTERY_VOLTAGE("Battery voltage is stable\n");
					break;
				}
				default: {
					LOG_BATTERY_VOLTAGE("Battery status is unknown\n");
					break;
				}
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
