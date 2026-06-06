/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Shock detector for senseGate
 * @author      Maverick Widjaja <Maverick.widjaja@haw-hamburg.de>
 */
#include "shock_detector.h"

static float calculate_magnitude(float x, float y, float z) {
	return sqrt(x * x + y * y + z * z);
}

static void acceleration_callback(void) {
	LOG_DEBUG("[shock_detector:%d] Shock!!\n",__LINE__);
}

static void* acceleration_thread(void* detector_void) {
	shock_detector_t* detector = (shock_detector_t*)detector_void;
	phydat_t acceleration;
	while (detector->running) {
		/* read an acceleration value from the sensor */
		int acc_dim = saul_reg_read(detector->accel_sensor, &acceleration);
		if (acc_dim < 1) {
			LOG_INFO("Error reading a value "
				 "from the device - %s:%d\n",__FILE__,__LINE__);
			return NULL;
		}
		float x = acceleration.val[0] / 100.0;
		float y = acceleration.val[1] / 100.0;
		float z = acceleration.val[2] / 100.0;

		float magnitude = calculate_magnitude(x, y, z);

		
		if (magnitude > detector->threshold) {
			//execute the callback here
			detector->callback();
			LED0_ON;
			LED1_ON;
		} else {
			LED0_OFF;
			LED1_OFF;
		}
		//LOG_DEBUG("[shock_detector:%d] x: %.2f, y: %.2f, z: %.2f, magnitude: %.2f\n", __LINE__, x, y, z, magnitude);
		ztimer_sleep(ZTIMER_MSEC, 100);
	}
	return NULL;
}

shock_detector_t* shock_detector_new(float threshold) {
	shock_detector_t* new_detector = (shock_detector_t*)malloc(sizeof(shock_detector_t));
	new_detector->running = false;
	new_detector->threshold = threshold;
	// sensor_data_t* accel_sensor = &new_detector->accel_sensor;
	// accel_sensor->callback = acceleration_callback;
	new_detector->callback = acceleration_callback;
	/* [TASK 3: find your device here] */
	new_detector->accel_sensor = saul_reg_find_type(SAUL_SENSE_ACCEL);
	if (!new_detector->accel_sensor) {
		LOG_DEBUG("[shock_detector:%d] No accelerometer sensor found!\n",__LINE__);
		return NULL;
	} else {
		//commented out for now to reduce console output, but can be useful for debugging
		LOG_DEBUG("[shock_detector:%d] Accelerometer sensor found: %s\n",__LINE__, new_detector->accel_sensor->name);
	}

	return new_detector;
}

int shock_detector_start(shock_detector_t* detector) {
	kernel_pid_t* accel_thread_pid = &detector->accel_thread_pid;
	detector->running = true;
	*accel_thread_pid = thread_create(detector->accel_thread_stack,
									  sizeof(detector->accel_thread_stack),
									  THREAD_PRIORITY_MAIN - 1,
									  THREAD_CREATE_STACKTEST,
									  acceleration_thread,
									  (void*) detector,
									  "Acceleration Thread");
	if (*accel_thread_pid == EOVERFLOW) {
		LOG_DEBUG("[shock_detector:%d] Error creating acceleration thread - %s:%d\n",__LINE__,__FILE__,__LINE__);
		return -1;
	}
	// ... and also commented out
	LOG_DEBUG("[shock_detector:%d] Acceleration thread started with PID %d\n",__LINE__, *accel_thread_pid);
	return 0;
}

int shock_detector_delete(shock_detector_t* detector) {
	detector->running = false;
	free(detector);
	return 0;
}
