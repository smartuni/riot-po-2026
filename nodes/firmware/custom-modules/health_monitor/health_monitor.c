#include "health_monitor.h"

// health_monitor_t* health_monitor_new(int low_battery_threshold_mv, int battery_update_period_sec, int shock_detector_update_period_sec) {
// 	health_monitor_t* instance = (health_monitor_t*)malloc(sizeof(health_monitor_t));
// 	instance->low_battery_threshold_mv = low_battery_threshold_mv;
// 	instance->battery_update_period_sec = battery_update_period_sec;
// 	instance->shock_detector_update_period_sec = shock_detector_update_period_sec;

// 	instance->is_low_battery = false;
// 	//init battery monitoring
// 	instance->battery_instance = battery_voltage_monitor_new();

	

// 	return instance;
// }

int health_monitor_init(health_monitor_t* instance, int low_battery_threshold_mv, int battery_update_period_sec, int shock_detector_update_period_sec){
	instance->low_battery_threshold_mv = low_battery_threshold_mv;
	instance->battery_update_period_sec = battery_update_period_sec;
	instance->is_low_battery = false;
	instance->battery_monitor_running = false;
	battery_voltage_monitor_init(&instance->battery_instance);

	instance->shock_detector_update_period_sec = shock_detector_update_period_sec;
	//init shock detection
	// int shock_threshold = 15000; // TODO adjust this later
	// int sampling_period_ms = 1; //to give other threads a chance to run
	// instance->shock_instance = shock_detector_new(shock_threshold, sampling_period_ms); // TODO adjust parameters later
	return 0;
}

static void serialize_and_send(const health_monitor_payload_t* payload) {
	//serialize the payload
	uint8_t buffer[HEALTH_MONITOR_CBOR_SIZE_BYTES];
	size_t buff_size = sizeof(buffer);
	health_monitor_serialize_record_no_sig(payload, buffer, &buff_size);
	LOG_DEBUG("[health_monitor.c:%d] Serialized health monitor payload, size: %d bytes\n", __LINE__, buff_size);
	for (size_t i = 0; i < buff_size; i++) {
		LOG_DEBUG("0x%02X ", buffer[i]);
	}
	LOG_DEBUG("\n");
	//send the payload via lorawan
	int status = send_lorawan_packet(buffer, buff_size);
	if (status == 0) {
		LOG_DEBUG("[health_monitor.c:%d] Sent health monitor payload via LoRaWAN\n", __LINE__);
	} else {
		LOG_DEBUG("[health_monitor.c:%d] Failed to send health monitor payload via LoRaWAN: lorawan is not initialized yet\n", __LINE__);
	}
}

static void* battery_function(void* instance_void) {
	//LOG_DEBUG("[health_monitor.c:%d] Starting the battery monitoring thread...\n", __LINE__);
	health_monitor_t* instance = (health_monitor_t*)instance_void;
	while (instance->battery_monitor_running) {
		//init the payload
		health_monitor_payload_t payload;

		battery_info_t battery_info = battery_voltage_monitor_fetch_info(&instance->battery_instance);
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
		serialize_and_send(&payload);

		// check for low battery and report if it's not yet reported
		if (battery_info.voltage_mv < instance->low_battery_threshold_mv && !instance->is_low_battery) {
			//report low battery one time and set the flag
			payload.header = BATTERY_LOW;
			payload.body = battery_info.voltage_mv;
			serialize_and_send(&payload);
			instance->is_low_battery = true;

			//reset the low battery flag after the battery voltage goes above the threshold
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
	// if (instance->shock_instance != NULL) {
	// 	shock_detector_start(instance->shock_instance);
	// } else {
	// 	return -1;
	// }

	LOG_DEBUG("[health_monitor.c:%d] Starting the battery monitoring thread...\n", __LINE__);
	instance->battery_monitor_running = true;
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