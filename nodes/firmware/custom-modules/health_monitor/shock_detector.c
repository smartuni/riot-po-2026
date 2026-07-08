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

static int calculate_magnitude(int x, int y, int z) {
	return sqrt(x * x + y * y + z * z);
}

static void shock_callback(void) {
	LOG_DEBUG("[shock_detector.c:%d] Shock!!\n", __LINE__);
}

static void collect_magnitudes(shock_detector_t* instance) {
	phydat_t acceleration;
	memset(instance->raw_accel_data, 0, sizeof(raw_acceleration_t) * instance->sample_size);
	LED1_ON;
	for (int i = 0; i < instance->sample_size; i++) {
		int acc_dim = saul_reg_read(instance->accel_sensor, &acceleration);
		if (acc_dim < 1) {
			LOG_INFO("[shock_detector.c:%d] Error reading a value "
					 "from the device\n",
					 __LINE__);
			return;
		}
		instance->raw_accel_data[i].x = acceleration.val[0] * 10;
		instance->raw_accel_data[i].y = acceleration.val[1] * 10;
		instance->raw_accel_data[i].z = acceleration.val[2] * 10;
		if (instance->sampling_period_ms > 0) {
			ztimer_sleep(ZTIMER_MSEC, instance->sampling_period_ms);
		}
	}
	LOG_DEBUG("[shock_detector.c:%d] Collected samples\n", __LINE__);

	memset(instance->input, 0, sizeof(kiss_fft_cpx) * instance->sample_size);

	for (int i = 0; i < instance->sample_size; i++) {
		int* x = &instance->raw_accel_data[i].x;
		int* y = &instance->raw_accel_data[i].y;
		int* z = &instance->raw_accel_data[i].z;
		instance->input[i].r = calculate_magnitude(*x, *y, *z);
		instance->input[i].i = 0;
	}
	LED1_OFF;
}

static void process_fft(shock_detector_t* instance) {
	memset(instance->output, 0, sizeof(kiss_fft_cpx) * instance->sample_size);
	kiss_fft(instance->cfg, instance->input, instance->output);
}

static void postprocess_fft(shock_detector_t* instance) {
	moving_freq_avg_reset(instance->freq_avg);
	for (int k = 0; k < instance->nyquist_domain_size; k++) {
		int magnitude = calculate_magnitude(instance->output[k].r, instance->output[k].i, 0);
		float sampling_rate_hz = 1000.0 / instance->sampling_period_ms;
		int frequency = k * ((float)sampling_rate_hz / instance->sample_size);
		if (magnitude > 0) {
			moving_freq_avg_add_sample(instance->freq_avg, frequency, magnitude);
		}
	}
	moving_freq_avg_finalize(instance->freq_avg);
}

static int find_variance(int data[], int n) {
    if (n <= 0) return 0;

    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    float mean = (float)sum / n;

    float sq_diff_sum = 0.0;
    for (int i = 0; i < n; i++) {
        float diff = data[i] - mean;
        sq_diff_sum += diff * diff;
    }

    float variance = sq_diff_sum / n;
    return (int)variance;
}

static bool check_shock(shock_detector_t* instance) {
	//it's 
	//frequency from 0 Hz to 94 Hz must all be above 40000 AND,
	//frequency from 95 Hz to 500 Hz must contain at least one value that is  below 25000, AND
	//frequency from 50 Hz to 95 Hz, if the difference between lowest value and the highest value is higher than 40000
	int frequency_point1 = 50;
	int frequency_point2 = 95;
	int magnitude_threshold_low = 44000;
	int magnitude_threshold_high = 25000;

	int lowest_value = INT_MAX;
	int highest_value = INT_MIN;
	int delta = 40000;

	for(int i = 5; i < frequency_point1; i++) {
		int* magnitude = &instance->freq_avg->frequency_domain[i].average;
		if(*magnitude <= lowest_value) {
			lowest_value = *magnitude;
		}
		if(*magnitude >= highest_value) {
			highest_value = *magnitude;
		}
	}

	if(highest_value - lowest_value < delta) {
		LOG_DEBUG("[shock_detector.c:%d] No shock detected: highest_value - lowest_value = %d < %d\n", __LINE__, highest_value - lowest_value, delta);
		return false;
	}

	for (int i = frequency_point1; i < frequency_point2; i++) {
		int* magnitude = &instance->freq_avg->frequency_domain[i].average;
		if (*magnitude > magnitude_threshold_low) {
			LOG_DEBUG("[shock_detector.c:%d] No shock detected: magnitude at frequency %d Hz = %d > %d\n", __LINE__, i, *magnitude, magnitude_threshold_low);
			return false;
		}
	}
	for (int i = frequency_point2; i < instance->nyquist_domain_size; i++) {
		int* magnitude = &instance->freq_avg->frequency_domain[i].average;
		if (*magnitude > magnitude_threshold_high) {
			LOG_DEBUG("[shock_detector.c:%d] No shock detected: magnitude at frequency %d Hz = %d > %d\n", __LINE__, i, *magnitude, magnitude_threshold_high);
			return false;
		}
	}
	return true;
}

static void* acceleration_thread(void* instance_void) {
	shock_detector_t* instance = (shock_detector_t*)instance_void;
	while (instance->running) {
		LOG_DEBUG("[shock_detector.c:%d] Collecting magnitudes...\n", __LINE__);
		collect_magnitudes(instance);
		LOG_DEBUG("[shock_detector.c:%d] Processing FFT...\n", __LINE__);
		process_fft(instance); // process the collected samples with FFT
		LOG_DEBUG("[shock_detector.c:%d] Post-processing FFT results...\n", __LINE__);
		postprocess_fft(instance); // post-process the FFT results to find the average over frequency
		LOG_DEBUG("[shock_detector.c:%d] Frequency ; Magnitude\n", __LINE__);
		for (int i = 0; i < instance->nyquist_domain_size; i += 5) {
			LOG_DEBUG("%d ; %d\n", i, instance->freq_avg->frequency_domain[i].average);
		}

		if (check_shock(instance)) {
			if (instance->callback) {
				instance->callback();
			} else {
				LOG_DEBUG("[shock_detector.c:%d] No callback function set for shock detection.\n", __LINE__);
			}
		} else {
			LOG_DEBUG("[shock_detector.c:%d] No shock detected.\n", __LINE__);
		}
		// mutex_lock(&instance->shock_status_mutex);
		// instance->shock_status = NO_SHOCK; //TODO analyze the frequency domain average to determine if there is a shock or not, and set the shock status accordingly
		// mutex_unlock(&instance->shock_status_mutex);
	}
	return NULL;
}

int shock_detector_init(shock_detector_t* instance, int threshold, int sampling_period_ms) {
	//instance = (shock_detector_t*)malloc(sizeof(shock_detector_t));
	instance->running = false;
	instance->threshold = threshold;
	instance->sample_size = SAMPLE_SIZE;
	instance->sampling_period_ms = sampling_period_ms;
	instance->callback = shock_callback;
	int nyquist = instance->sample_size / 2 + 1;
	instance->nyquist_domain_size = nyquist; // + 1;
	instance->freq_avg = moving_freq_avg_new(instance->nyquist_domain_size);
	instance->cfg = kiss_fft_alloc(instance->sample_size, 0, NULL, NULL);
	instance->shock_status = NO_SHOCK;
	instance->shock_status_mutex = (mutex_t)MUTEX_INIT;

	/* [TASK 3: find your device here] */
	instance->accel_sensor = saul_reg_find_type(SAUL_SENSE_ACCEL);
	if (!instance->accel_sensor) {
		LOG_DEBUG("[shock_detector:%d] No accelerometer sensor found!\n", __LINE__);
		return -1;
	} else {
		//commented out for now to reduce console output, but can be useful for debugging
		LOG_DEBUG("[shock_detector:%d] Accelerometer sensor found: %s\n", __LINE__, instance->accel_sensor->name);
	}

	return 0;
}

int shock_detector_start(shock_detector_t* instance) {
	kernel_pid_t* accel_thread_pid = &instance->thread_pid;
	instance->running = true;
	*accel_thread_pid = thread_create(instance->accel_thread_stack,
									  sizeof(instance->accel_thread_stack),
									  THREAD_PRIORITY_MAIN - 1,
									  THREAD_CREATE_STACKTEST,
									  acceleration_thread,
									  (void*)instance,
									  "Acceleration Thread");
	if (*accel_thread_pid == EOVERFLOW) {
		LOG_DEBUG("[shock_detector:%d] Error creating acceleration thread - %s:%d\n", __LINE__, __FILE__, __LINE__);
		return -1;
	}
	// ... and also commented out
	LOG_DEBUG("[shock_detector:%d] Acceleration thread started with PID %d\n", __LINE__, *accel_thread_pid);
	return 0;
}

int shock_detector_delete(shock_detector_t* instance) {
	instance->running = false;
	ztimer_sleep(ZTIMER_MSEC, 5000); // give some time for the thread to exit
	moving_freq_avg_delete(instance->freq_avg);
	kiss_fft_free(instance->cfg);
	return 0;
}

int shock_detector_fetch_status(shock_detector_t* instance, shock_status_t* status) {
	mutex_lock(&instance->shock_status_mutex);
	*status = instance->shock_status;
	mutex_unlock(&instance->shock_status_mutex);
	return 0;
}

int shock_detector_reset_status(shock_detector_t* instance) {
	mutex_lock(&instance->shock_status_mutex);
	instance->shock_status = NO_SHOCK;
	mutex_unlock(&instance->shock_status_mutex);
	return 0;
}