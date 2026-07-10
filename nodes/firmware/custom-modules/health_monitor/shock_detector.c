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

static void collect_magnitudes(shock_detector_t* instance) {
	phydat_t acceleration;
	memset(instance->raw_accel_data, 0, sizeof(raw_acceleration_t) * instance->sample_size);
	// LED1_ON;
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
	memset(instance->input, 0, sizeof(kiss_fft_cpx) * instance->sample_size);

	for (int i = 0; i < instance->sample_size; i++) {
		int* x = &instance->raw_accel_data[i].x;
		int* y = &instance->raw_accel_data[i].y;
		int* z = &instance->raw_accel_data[i].z;
		instance->input[i].r = calculate_magnitude(*x, *y, *z);
		instance->input[i].i = 0;
	}
	//LED1_OFF;
}

static void process_fft(shock_detector_t* instance) {
	memset(instance->output, 0, sizeof(kiss_fft_cpx) * instance->sample_size);
	kiss_fft(instance->cfg, instance->smoothed_input, instance->output);
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

static bool check_free_fall(shock_detector_t* instance) {
	for (int i = 0; i < instance->sample_size; i+=3) {
		if(instance->input[i].r < FREE_FALL_THRESHOLD) {
			return true;
		}
	}
	return false;
}

static bool check_shock(shock_detector_t* instance) {

	//for the values below 100hz: count zeroes in the frequency domain, if more than SHOCK_THRESHOLD zeroes, then return true
	int zeroes_count = 0;
	for(int i = 0; i < 100; i+= 3) {
		if(instance->freq_avg->frequency_domain[i].average == 0) {
			zeroes_count++;
		}
	}
	if(zeroes_count > SHOCK_THRESHOLD) {
		return true;
	}
	return false;
}


static void sliding_avg(kiss_fft_cpx* data, int data_len, int window, kiss_fft_cpx* result, int* result_len) {    
    *result_len = data_len - window + 1;
        
    // Calculate first window sum
    int sum = 0;
    for (int i = 0; i < window; i++) {
        sum += data[i].r;
    }
    result[0].r = (int) (sum / window);
    
    // Slide the window
    for (int i = 1; i < *result_len; i++) {
        sum = sum - data[i - 1].r + data[i + window - 1].r;
        result[i].r = (int) (sum / window);
    }

}

static void* acceleration_thread(void* instance_void) {
	shock_detector_t* instance = (shock_detector_t*)instance_void;
	while (instance->running) {
		//collect the accelerometer data
		collect_magnitudes(instance);
		//convolve the data to remove the noise
		sliding_avg(instance->input, instance->sample_size, AVG_SLIDING_WINDOW, instance->smoothed_input, &instance->smoothed_size);
		// process the collected samples with FFT
		process_fft(instance); 
		// post-process the FFT results to round the frequency domain and calculate the average magnitude for each frequency
		postprocess_fft(instance);

		//to read the convolved accelerometer data, uncomment the following lines
		//LOG_DEBUG("[shock_detector.c:%d] Frequency ; Magnitude\n", __LINE__);
		// for (int i = 0; i < instance->nyquist_domain_size; i += 3) {
		// 	LOG_DEBUG(" ; %d ; %d\n", i, instance->freq_avg->frequency_domain[i].average);
		// }

		//to perform calibration, uncomment the following lines and plot them to csv to see the frequency domain and the magnitude of each frequency
		//LOG_DEBUG("[shock_detector.c:%d] Sample ; Magnitude\n", __LINE__);
		// for (int i = 0; i < instance->sample_size; i += 3) {
		// 	LOG_DEBUG(" ; %d ; %d\n", i, (int)(instance->input[i].r) - ACCELEROMETER_EARTH_GRAVITY);
		// }
		
		LOG_DEBUG("[shock_detector.c:%d] Checking accelerometer data...\n", __LINE__);
		if(check_free_fall(instance)) {
			mutex_lock(&instance->shock_status_mutex);
			instance->shock_status = FREE_FALL;
			mutex_unlock(&instance->shock_status_mutex);
			sem_post(&instance->shock_count_to_report);
			LOG_DEBUG("[shock_detector.c:%d] Free fall detected.\n", __LINE__);
		} 
		if(check_shock(instance)){
			mutex_lock(&instance->shock_status_mutex);
			instance->shock_status = SHOCK_DETECTED;
			mutex_unlock(&instance->shock_status_mutex);
			sem_post(&instance->shock_count_to_report);
			LOG_DEBUG("[shock_detector.c:%d] Shock detected.\n", __LINE__);
		}
	}
	return NULL;
}

int shock_detector_init(shock_detector_t* instance, int sampling_period_ms) {
	instance->shock_status = NO_SHOCK;
	instance->running = true;
	instance->sample_size = SAMPLE_SIZE;
	instance->sampling_period_ms = sampling_period_ms;
	int nyquist = instance->sample_size / 2 + 1;
	instance->nyquist_domain_size = nyquist; // + 1;
	instance->freq_avg = moving_freq_avg_new(instance->nyquist_domain_size);
	instance->cfg = kiss_fft_alloc(instance->sample_size, 0, NULL, NULL);
	instance->shock_status_mutex = (mutex_t)MUTEX_INIT;
	sem_init(&instance->shock_count_to_report, 0, 0);

	instance->accel_sensor = saul_reg_find_type(SAUL_SENSE_ACCEL);
	if (!instance->accel_sensor) {
		LOG_DEBUG("[shock_detector:%d] No accelerometer sensor found!\n", __LINE__);
		return -1;
	} else {
		LOG_DEBUG("[shock_detector:%d] Accelerometer sensor found: %s\n", __LINE__, instance->accel_sensor->name);
	}

	return 0;
}

int shock_detector_start(shock_detector_t* instance) {
	kernel_pid_t* accel_thread_pid = &instance->thread_pid;
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
	LOG_DEBUG("[shock_detector:%d] Acceleration thread started with PID %d\n", __LINE__, *accel_thread_pid);
	return 0;
}

int shock_detector_delete(shock_detector_t* instance) {
	instance->running = false;
	ztimer_sleep(ZTIMER_MSEC, 5000); // give some time for the thread to exit
	moving_freq_avg_delete(instance->freq_avg);
	kiss_fft_free(instance->cfg);
	sem_destroy(&instance->shock_count_to_report);
	return 0;
}


shock_status_t shock_detector_wait_for_shock(shock_detector_t* instance) {
	sem_wait(&instance->shock_count_to_report);
	mutex_lock(&instance->shock_status_mutex);
	shock_status_t status = instance->shock_status;
	mutex_unlock(&instance->shock_status_mutex);
	return status;
}