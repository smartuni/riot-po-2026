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
#include "board.h"

static int calculate_magnitude(int x, int y, int z) {
	return sqrt(x * x + y * y + z * z);
}

static void acceleration_callback(void) {
	LOG_DEBUG("[shock_detector:%d] Shock!!\n", __LINE__);
}

static void collect_samples(shock_detector_t* detector, int sampling_rate_hz) {
	phydat_t acceleration;
	int nsamples = detector->sample_size;
	LED0_ON;
	for (int i = 0; i < nsamples; i++) {
		int acc_dim = saul_reg_read(detector->accel_sensor, &acceleration);
		if (acc_dim < 1) {
			LOG_INFO("Error reading a value "
					 "from the device - %s:%d\n",
					 __FILE__, __LINE__);
			return;
		}
		int x = acceleration.val[0] * 10;
		int y = acceleration.val[1] * 10;
		int z = acceleration.val[2] * 10;
		//detector->sample_array[i] = calculate_magnitude(x, y, z);
		detector->input[i].r = calculate_magnitude(x, y, z);
		detector->input[i].i = 0;
		ztimer_sleep(ZTIMER_MSEC, 1000 / sampling_rate_hz);
	}
	LED0_OFF;
}

static void process_fft(shock_detector_t* detector) {
	kiss_fft_cfg cfg = kiss_fft_alloc(detector->sample_size, 0, 0, 0);
	kiss_fft(cfg, detector->input, detector->output);
	kiss_fft_free(cfg);
}

static void display_fft_results(shock_detector_t* detector) {
	printf("Frequency Spectrum (0 to %d Hz):\n", detector->sample_size / 2);
	printf("============================================================\n");
	printf("%-10s %-15s %-15s %-15s\n", "Bin", "Frequency(Hz)", "Magnitude", "Real/Imag");
	printf("============================================================\n");

	// Only need first N_SAMPLES/2 + 1 bins (Nyquist limit)
	// For N=500, bins 0 to 250 represent 0 to 500 Hz
	int nyquist_bin = detector->sample_size / 2;

	for (int k = 0; k <= nyquist_bin; k++) {
		// Calculate magnitude: sqrt(real^2 + imag^2)
		int magnitude = sqrt(detector->output[k].r * detector->output[k].r +
								detector->output[k].i * detector->output[k].i);

		// Calculate frequency for this bin
		int frequency = k * ((double)detector->sampling_rate_hz / detector->sample_size);

		// Only print bins with significant magnitude (above noise floor)
		// For demonstration, print everything but you'd typically filter
		if (magnitude > 0.1) {
			printf("%10d %10d %10d (%6d + %6di)\n",
				   k, frequency, magnitude, (int) detector->output[k].r, (int) detector->output[k].i);
		}
	}
}

static void* acceleration_thread(void* detector_void) {
	shock_detector_t* detector = (shock_detector_t*)detector_void;
	//phydat_t acceleration;
	while (detector->running) {
		int sampling_rate_hz = 100; // 100 Hz sampling rate
		collect_samples(detector, sampling_rate_hz); // collect samples at 100 Hz
		process_fft(detector); // process the collected samples with FFT

		display_fft_results(detector); // display the FFT results
		// if (magnitude > detector->threshold) {
		// 	//execute the callback here
		// 	detector->callback();
		// 	LED0_ON;
		// 	LED1_ON;
		// } else {
		// 	LED0_OFF;
		// 	LED1_OFF;
		// }
		//LOG_DEBUG("[shock_detector:%d] x: %5d, y: %5d, z: %5d, magnitude: %5d\n", __LINE__, x, y, z, magnitude);
		ztimer_sleep(ZTIMER_MSEC, 5000);
	}
	return NULL;
}

shock_detector_t* shock_detector_new(int threshold, int sample_size, int sampling_rate_hz) {
	shock_detector_t* new_detector = (shock_detector_t*)malloc(sizeof(shock_detector_t));
	new_detector->running = false;
	new_detector->threshold = threshold;
	new_detector->sample_size = sample_size;
	new_detector->sampling_rate_hz = sampling_rate_hz;
	// sensor_data_t* accel_sensor = &new_detector->accel_sensor;
	// accel_sensor->callback = acceleration_callback;
	new_detector->callback = acceleration_callback;
	new_detector->sample_array = (int*)malloc(sizeof(int) * sample_size);
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
	ztimer_sleep(ZTIMER_MSEC, 100); // give some time for the thread to exit
	free(detector->input);
	free(detector->output);
	free(detector->sample_array);
	free(detector);
	return 0;
}
