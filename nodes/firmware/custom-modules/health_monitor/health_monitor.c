#include "health_monitor.h"

health_monitor_t* health_monitor_new(int update_period_sec) {
	health_monitor_t* monitor = malloc(sizeof(health_monitor_t));
	monitor->update_period_sec = update_period_sec;

	//init battery monitoring
	int threshold_mv = 3000; // TODO adjust this later
	monitor->battery_monitor = battery_voltage_monitor_new(threshold_mv);

	//init shock detection
	int shock_threshold = 15000; // TODO adjust this later
	int sample_size = 2048; // TODO adjust this later
	int sampling_period_ms = 1;
	monitor->shock_detector = shock_detector_new(shock_threshold, sample_size, sampling_period_ms); // TODO adjust parameters later

	return monitor;
}

static void* thread_function(void* monitor_void) {
	health_monitor_t* monitor = (health_monitor_t*)monitor_void;
	while (monitor->running) {
		//get battery info
		//get shock status
		ztimer_sleep(ZTIMER_SEC, monitor->update_period_sec);
	}
	return NULL;
}

int health_monitor_start(health_monitor_t* monitor) {
	if (monitor == NULL) {
		return -1;
	}
	//start shock detection
	if (monitor->shock_detector) {
		shock_detector_start(monitor->shock_detector);
	} else {
		return -1;
	}

	monitor->thread_pid = thread_create(monitor->thread_stack,
										sizeof(monitor->thread_stack),
										THREAD_PRIORITY_MAIN - 1,
										THREAD_CREATE_STACKTEST,
										thread_function,
										(void*)monitor, "Health Monitor Thread");

	return 0;
}

int health_monitor_delete(health_monitor_t* monitor) {
	if (monitor == NULL) {
		return -1;
	}
	shock_detector_delete(monitor->shock_detector);
	free(monitor->shock_detector);
	battery_voltage_monitor_delete(monitor->battery_monitor);
	free(monitor->battery_monitor);
	free(monitor);
	return 0;
}