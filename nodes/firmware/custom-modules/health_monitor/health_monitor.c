#include "health_monitor.h"

health_monitor_t* health_monitor_new(void) {
	health_monitor_t* instance = (health_monitor_t*)malloc(sizeof(health_monitor_t));
	instance->update_period_sec = 10; // default update period
	instance->low_battery_threshold_mv = 3700; // default low battery threshold
	instance->is_low_battery = false;
	instance->battery_update_period_sec = 10; // default battery update period

	//init battery monitoring
	instance->battery_monitor = battery_voltage_monitor_new();

	//init shock detection
	// int shock_threshold = 15000; // TODO adjust this later
	// int sampling_period_ms = 1; //to give other threads a chance to run
	// instance->shock_detector = shock_detector_new(shock_threshold, sampling_period_ms); // TODO adjust parameters later

	return instance;
}

static void serialize_and_send(const health_monitor_payload_t* payload) {
	//serialize the payload
	uint8_t buffer[HEALTH_MONITOR_BUFFER_SIZE];
	size_t buff_size = sizeof(buffer);
	health_monitor_serialize_record_no_sig(payload, buffer, &buff_size);
	LOG_DEBUG("[health_monitor.c:%d] Serialized health monitor payload, size: %d bytes\n", __LINE__, buff_size);
	for (size_t i = 0; i < buff_size; i++) {
		LOG_DEBUG("0x%02X\n", buffer[i]);
	}
	//send the payload via lorawan
	int status = send_lorawan_packet(buffer, buff_size);
	if (status == 0) {
		LOG_DEBUG("[health_monitor.c:%d] Sent health monitor payload via LoRaWAN\n", __LINE__);
	} else {
		LOG_DEBUG("[health_monitor.c:%d] Failed to send health monitor payload via LoRaWAN: lorawan is not initialized yet\n", __LINE__);
	}
}

static void* battery_function(void* instance_void) {
	health_monitor_t* instance = (health_monitor_t*)instance_void;
	while (instance->running) {
		//init the payload
		health_monitor_payload_t payload;

		//TODO get battery info
		battery_info_t battery_info = battery_voltage_monitor_fetch_info(instance->battery_monitor);
		switch (battery_info.battery_status) {
			case CHARGING:
				payload.header = BATTERY_CHARGING;
				break;
			case DISCHARGING:
				payload.header = BATTERY_DISCHARGING;
				break;
			default:
				payload.header = BATTERY_DISCHARGING; // default to discharging if unknown
				break;
		}
		//report the battery state
		serialize_and_send(&payload);

		if (battery_info.voltage_mv < instance->low_battery_threshold_mv && !instance->is_low_battery) {
			instance->is_low_battery = true;
			//report low battery even
			payload.header = BATTERY_LOW;
			serialize_and_send(&payload);
		} else if (battery_info.voltage_mv >= instance->low_battery_threshold_mv && instance->is_low_battery) {
			instance->is_low_battery = false;
		}
		ztimer_sleep(ZTIMER_SEC, instance->battery_update_period_sec);
	}
	return NULL;
}

int health_monitor_start(health_monitor_t* instance) {
	if (instance == NULL) {
		return -1;
	}
	//start shock detection
	if (instance->shock_detector) {
		shock_detector_start(instance->shock_detector);
	} else {
		return -1;
	}

	instance->battery_thread_pid = thread_create(instance->battery_thread_stack,
										sizeof(instance->battery_thread_stack),
										THREAD_PRIORITY_MAIN - 1,
										THREAD_CREATE_STACKTEST,
										battery_function,
										(void*)instance, "Battery Thread");

	return 0;
}

int health_monitor_delete(health_monitor_t* instance) {
	if (instance == NULL) {
		return -1;
	}
	// shock_detector_delete(instance->shock_detector);
	// free(instance->shock_detector);
	// battery_voltage_monitor_delete(instance->battery_monitor);
	// free(instance->battery_monitor);
	// free(instance);
	return 0;
}