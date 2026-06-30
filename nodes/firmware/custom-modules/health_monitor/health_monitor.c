#include "health_monitor.h"

health_monitor_t* health_monitor_new(void) {
	health_monitor_t* monitor = malloc(sizeof(health_monitor_t));

	//int threshold_mv = 3000; // TODO adjust this later
	//init battery monitoring
	//monitor->battery_monitor = battery_voltage_monitor_new(threshold_mv); // TODO adjust threshold later

	//init shock detection
	//int shock_threshold = 15000; // TODO adjust this later
	//int sampling_period_ms = 1; //to give other threads a chance to run
	//monitor->shock_detector = shock_detector_new(shock_threshold, sampling_period_ms); // TODO adjust parameters later

	return monitor;
}

int health_monitor_start(health_monitor_t* monitor) {
	if (monitor == NULL) {
		return -1;
	}
	//start battery monitoring
	// if (monitor->battery_monitor) {
	// 	battery_voltage_monitor_start(monitor->battery_monitor);
	// } else {
	// 	return -1;
	// }

	//start shock detection
	//if (monitor->shock_detector) {
	//shock_detector_start(monitor->shock_detector);
	//} else {
	//return -1;
	//}

	return 0;
}

int health_monitor_stop(health_monitor_t* monitor) {
	if (monitor == NULL) {

		return -1;
	}
	// printf(" health monitor is stopped.\n");
	return 0;
}