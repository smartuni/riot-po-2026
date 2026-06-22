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

static void acceleration_callback(void) {
	LOG_DEBUG("[shock_detector.c:%d] Shock!!\n", __LINE__);
}

static void collect_magnitudes(shock_detector_t* detector) {
	phydat_t acceleration;
	const int* nsamples = &detector->sample_size;
	raw_acceleration_t* raw_accel_data = (raw_acceleration_t*)malloc(sizeof(raw_acceleration_t) * (*nsamples));
	LED0_ON;
	for (int i = 0; i < *nsamples; i++) {
		int acc_dim = saul_reg_read(detector->accel_sensor, &acceleration);
		if (acc_dim < 1) {
			LOG_INFO("[shock_detector.c:%d] Error reading a value "
					 "from the device\n",
					 __LINE__);
			return;
		}
		raw_accel_data[i].x = acceleration.val[0] * 10;
		raw_accel_data[i].y = acceleration.val[1] * 10;
		raw_accel_data[i].z = acceleration.val[2] * 10;
		if (detector->sampling_period_ms > 0) {
			ztimer_sleep(ZTIMER_MSEC, detector->sampling_period_ms);
		}
	}

	for (int i = 0; i < *nsamples; i++) {
		detector->input[i].r = 0;
	}
	for (int i = 0; i < *nsamples; i++) {
		int* x = &raw_accel_data[i].x;
		int* y = &raw_accel_data[i].y;
		int* z = &raw_accel_data[i].z;
		detector->input[i].r = calculate_magnitude(*x, *y, *z);
		detector->input[i].i = 0;
	}
	free(raw_accel_data);
	LED0_OFF;
}

static void process_fft(shock_detector_t* detector) {
	LED1_ON;
	for (int i = 0; i < detector->sample_size; i++) {
		detector->output[i].r = 0;
		detector->output[i].i = 0;
	}
	kiss_fft_cfg cfg = kiss_fft_alloc(detector->sample_size, 0, 0, 0);
	kiss_fft(cfg, detector->input, detector->output);
	kiss_fft_free(cfg);
	LED1_OFF;
}

static void postprocess_fft(shock_detector_t* detector) {
	moving_freq_avg_reset(detector->freq_avg);
	for (int k = 0; k < detector->nyquist_domain_size; k++) {
		int magnitude = calculate_magnitude(detector->output[k].r, detector->output[k].i, 0);
		float sampling_rate_hz = 1000.0 / detector->sampling_period_ms;
		int frequency = k * ((float)sampling_rate_hz / detector->sample_size);
		if (magnitude > 0) {
			moving_freq_avg_add_sample(detector->freq_avg, frequency, magnitude);
		}
	}
	moving_freq_avg_finalize(detector->freq_avg);
}

static void* acceleration_thread(void* detector_void) {
	shock_detector_t* detector = (shock_detector_t*)detector_void;
	while (detector->running) {
		LOG_DEBUG("[shock_detector.c:%d] Collecting magnitudes...\n", __LINE__);
		collect_magnitudes(detector);
		LOG_DEBUG("[shock_detector.c:%d] Processing FFT...\n", __LINE__);
		process_fft(detector); // process the collected samples with FFT
		LOG_DEBUG("[shock_detector.c:%d] Post-processing FFT results...\n", __LINE__);
		postprocess_fft(detector); // post-process the FFT results to find the average over frequency
		for (int i = 0; i < 500; i += 2) {
			LOG_DEBUG("[shock_detector.c:%d] Frequency: %d Hz, Average Magnitude: %d\n", __LINE__, i, detector->freq_avg->frequency_domain[i].average);
		}
	}
	return NULL;
}

shock_detector_t* shock_detector_new(int threshold, int sample_size, int sampling_period_ms) {
	shock_detector_t* new_detector = (shock_detector_t*)malloc(sizeof(shock_detector_t));
	new_detector->running = false;
	new_detector->threshold = threshold;
	new_detector->sample_size = sample_size;
	new_detector->sampling_period_ms = sampling_period_ms;
	// sensor_data_t* accel_sensor = &new_detector->accel_sensor;
	// accel_sensor->callback = acceleration_callback;
	new_detector->callback = acceleration_callback;
	int nyquist = sample_size / 2 + 1;
	new_detector->nyquist_domain_size = nyquist; // + 1;
	new_detector->freq_avg = moving_freq_avg_new(new_detector->nyquist_domain_size);
	new_detector->input = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * sample_size);
	new_detector->output = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * sample_size);

	/* [TASK 3: find your device here] */
	new_detector->accel_sensor = saul_reg_find_type(SAUL_SENSE_ACCEL);
	if (!new_detector->accel_sensor) {
		LOG_DEBUG("[shock_detector:%d] No accelerometer sensor found!\n", __LINE__);
		return NULL;
	} else {
		//commented out for now to reduce console output, but can be useful for debugging
		LOG_DEBUG("[shock_detector:%d] Accelerometer sensor found: %s\n", __LINE__, new_detector->accel_sensor->name);
	}

	return new_detector;
}

int shock_detector_start(shock_detector_t* detector) {
	kernel_pid_t* accel_thread_pid = &detector->thread_pid;
	detector->running = true;
	*accel_thread_pid = thread_create(detector->accel_thread_stack,
									  sizeof(detector->accel_thread_stack),
									  THREAD_PRIORITY_MAIN - 1,
									  THREAD_CREATE_STACKTEST,
									  acceleration_thread,
									  (void*)detector,
									  "Acceleration Thread");
	if (*accel_thread_pid == EOVERFLOW) {
		LOG_DEBUG("[shock_detector:%d] Error creating acceleration thread - %s:%d\n", __LINE__, __FILE__, __LINE__);
		return -1;
	}
	// ... and also commented out
	LOG_DEBUG("[shock_detector:%d] Acceleration thread started with PID %d\n", __LINE__, *accel_thread_pid);
	return 0;
}

int shock_detector_delete(shock_detector_t* detector) {
	detector->running = false;
	ztimer_sleep(ZTIMER_MSEC, 5000); // give some time for the thread to exit
	free(detector->input);
	free(detector->output);
	moving_freq_avg_delete(detector->freq_avg);
	free(detector);
	return 0;
}
