#include "health_monitor.h"

health_monitor_t* health_monitor_new(int update_period_sec) {
	health_monitor_t* monitor = malloc(sizeof(health_monitor_t));
	monitor->update_period_sec = update_period_sec;

	//init battery monitoring
	int threshold_mv = 3000; // TODO adjust this later
	monitor->battery_monitor = battery_voltage_monitor_new(threshold_mv);

	//init shock detection
	int shock_threshold = 15000; // TODO adjust this later
	int sampling_period_ms = 1; //to give other threads a chance to run
	monitor->shock_detector = shock_detector_new(shock_threshold, sampling_period_ms); // TODO adjust parameters later

	return monitor;
}

static void* thread_function(void* monitor_void) {
	health_monitor_t* monitor = (health_monitor_t*)monitor_void;
	while (monitor->running) {
		//init the payload
		health_monitor_payload_t payload;

		//TODO get battery info
		//battery_voltage_monitor_fetch_info(monitor->battery_monitor, &payload.battery_info);

		//TODO get shock status
		//shock_detector_fetch_status(monitor->shock_detector, &payload.shock_status);

		//serialize the payload
		uint8_t buffer[HEALTH_MONITOR_BUFFER_SIZE];
		size_t buff_size = sizeof(buffer);
		health_monitor_serialize_record_no_sig(&payload, buffer, &buff_size);
		LOG_DEBUG("[health_monitor.c:%d] Serialized health monitor payload, size: %d bytes\n", __LINE__, buff_size);

		//send the payload via lorawan	
		int status = send_lorawan_packet(buffer, buff_size);
		if (status == 0) {
			LOG_DEBUG("[health_monitor.c:%d] Sent health monitor payload via LoRaWAN\n", __LINE__);
		} else {
			LOG_DEBUG("[health_monitor.c:%d] Failed to send health monitor payload via LoRaWAN: lorawan is not initialized yet\n", __LINE__);
		}
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