#include "health_monitor.h"




static int serialize_and_send(const health_monitor_payload_t* payload) {
	uint8_t serialized_buffer[HEALTH_MONITOR_CBOR_SIZE_BYTES];
	size_t buff_size = sizeof(serialized_buffer);
	health_monitor_serialize(payload, serialized_buffer, &buff_size);
	int status = send_lorawan_packet(serialized_buffer, buff_size);
	return status;
}

static void* thread_battery_function(void* instance_void) {
	battery_voltage_monitor_t* instance = (battery_voltage_monitor_t*)instance_void;
	int low_battery_threshold_mv = LOW_BATTERY_THRESHOLD_MV;
	int battery_update_period_sec = BATTERY_UPDATE_PERIOD_SEC;
	bool is_low_battery = false;
	while (true) {
		health_monitor_payload_t payload;
		battery_info_t battery_info;

		battery_voltage_monitor_fetch_info(instance, &battery_info);
		switch (battery_info.battery_status) {
			case BATTERY_STATE_CHARGING:
				payload.header = BATTERY_CHARGING;
				break;
			case BATTERY_STATE_DISCHARGING:
				payload.header = BATTERY_DISCHARGING;
				break;
			default:
				payload.header = BATTERY_DISCHARGING; // default to discharging if unknown
				break;
		}
		payload.body = battery_info.voltage_mv;

		//report the battery state
		if(serialize_and_send(&payload) != 0) {
			LOG_ERROR("[health_monitor.c:%d] Failed to send battery status payload\n", __LINE__);
		} else {
			LOG_DEBUG("[health_monitor.c:%d] Sent battery status payload: header=%d, body=%d\n", __LINE__, payload.header, payload.body);
		}

		// check for low battery and report if it's not yet reported
		if (battery_info.voltage_mv < low_battery_threshold_mv && !is_low_battery) {
			//report low battery one time and set the flag
			payload.header = BATTERY_LOW;
			payload.body = battery_info.voltage_mv;
			if(serialize_and_send(&payload) != 0) {
				LOG_ERROR("[health_monitor.c:%d] Failed to send low battery payload\n", __LINE__);
			} else {
				LOG_DEBUG("[health_monitor.c:%d] Sent low battery payload: header=%d, body=%d\n", __LINE__, payload.header, payload.body);
			}
			is_low_battery = true;

			//reset the low battery flag after the battery voltage goes above the threshold
		} else if (battery_info.voltage_mv >= low_battery_threshold_mv && is_low_battery) {
			is_low_battery = false;
		}
		ztimer_sleep(ZTIMER_SEC, battery_update_period_sec);
	}
	return NULL;
}

static void* thread_shock_detector_function(void* instance_void) {
	shock_detector_t* instance = (shock_detector_t*)instance_void;
	shock_detector_start(instance);
	while (instance->running) {
		LOG_DEBUG("[health_monitor.c:%d] Waiting for shock detection...\n", __LINE__);
		health_monitor_payload_t payload;
		payload.header = ACCELEROMETER;
		payload.body = (int16_t) shock_detector_wait_for_accel_sig(instance);
	
		if(serialize_and_send(&payload) != 0) {
			LOG_ERROR("[health_monitor.c:%d] Failed to send accelerometer payload\n", __LINE__);
		} else {
			LOG_DEBUG("[health_monitor.c:%d] Sent accelerometer payload: header=%d, body=%d\n", __LINE__, payload.header, payload.body);
		}
	}
	return NULL;
}

int health_monitor_init(health_monitor_t* instance) {
	assert(instance != NULL);
	int result;
	result = battery_voltage_monitor_init(&instance->battery_instance);
	if(result != 0) {
		return result;
	}
	result = shock_detector_init(&instance->shock_detector_instance, 1);
	if(result != 0) {
		return result;
	}

	instance->shock_detector_thread_pid = thread_create(instance->shock_detector_thread_stack,
														sizeof(instance->shock_detector_thread_stack),
														THREAD_PRIORITY_MAIN - 1,
														THREAD_CREATE_STACKTEST,
														thread_shock_detector_function,
														(void*) &instance->shock_detector_instance, "Shock Detector Thread");
	instance->battery_thread_pid = thread_create(instance->battery_thread_stack,
												 sizeof(instance->battery_thread_stack),
												 THREAD_PRIORITY_MAIN - 1,
												 THREAD_CREATE_STACKTEST,
												 thread_battery_function,
												 (void*) &instance->battery_instance, "Battery Thread");

	return 0;
}
