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

	//for the values below 100hz: if there's at least one value, which is lower than 10000, return true, else return false
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




	// float sum = 0.0;
	// int accuracy = 1;
	// float mean = 0.0;
	// float sq_diff_sum_for_stddev = 0.0;
	// int zeroes_count = 0;
	// float variance = 0.0;
	// float stddev = 0.0;
	// float sq_sum = 0.0; // Renamed from sq_diff_sum

	// float max_value = 0.0; // Changed from INT_MIN since values are float

	// //BEGIN VARIANCE CHECK
	// int lower_limit = 0; // Hz - only check variance for frequencies above this limit
	// int upper_limit = 30; // Hz - only check variance for frequencies below this limit
	// int count = 0; // Actual number of samples processed

	// for (int i = lower_limit; i < upper_limit; i += accuracy) {
	// 	float current_value = instance->freq_avg->frequency_domain[i].average;
	// 	sum += current_value;
	// 	sq_sum += current_value * current_value; // For RMS calculation

	// 	if (current_value == 0.0f) {
	// 		zeroes_count++;
	// 	}
	// 	if (current_value > max_value) {
	// 		max_value = current_value;
	// 	}
	// 	count++;
	// }

	// mean = sum / count;

	// // Calculate variance and standard deviation
	// for (int i = lower_limit; i < upper_limit; i += accuracy) {
	// 	float diff = instance->freq_avg->frequency_domain[i].average - mean;
	// 	sq_diff_sum_for_stddev += diff * diff;
	// }

	// // Calculate Spectral Crest Factor (SCF) = max_value / RMS
	// // RMS = sqrt(mean of squared values)
	// float mean_square = sq_sum / count;
	// float rms = sqrt(mean_square);

	// if (rms == 0.0f) {
	// 	LOG_DEBUG("[shock_detector.c:%d] RMS is zero, cannot calculate SCF\n", __LINE__);
	// 	return false;
	// }

	// float scf = max_value / rms;

	// variance = sq_diff_sum_for_stddev / count;
	// stddev = sqrt(variance);

	// LOG_DEBUG("[shock_detector.c:%d] %d-%d Hz: Mean:%d, StdDev:%d, Zeros:%d, SCF:%d\n",
	// 		  __LINE__, lower_limit, upper_limit, (int)mean, (int)stddev, zeroes_count, (int)scf);
	// //END VARIANCE CHECK

	// // Add your classification logic here based on SCF and/or zeroes_count
	// // For example:
	// // if (scf > SOME_THRESHOLD || zeroes_count > SOME_THRESHOLD) return true;

	// return false;
}

// static bool debug_shock(shock_detector_t* instance) {

// 	int variance = 0;
// 	int sum = 0;
// 	int accuracy = 1;
// 	float mean = 0.0;
// 	float sq_diff_sum = 0.0;
// 	int zeroes_count = 0;

// 	//BEGIN VARIANCE CHECK
// 	variance = 0;
// 	sum = 0;
// 	int lower_limit = 5; // only check variance for frequencies above this limit
// 	int upper_limit = 50; // only check variance for frequencies below this limit
// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		sum += instance->freq_avg->frequency_domain[i].average;
// 		if (instance->freq_avg->frequency_domain[i].average == 0) {
// 			zeroes_count++;
// 		}
// 	}
// 	mean = (float)sum / (upper_limit - lower_limit);

// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		float diff = instance->freq_avg->frequency_domain[i].average - mean;
// 		sq_diff_sum += diff * diff;

// 	}
// 	variance = (int)(sq_diff_sum / (upper_limit - lower_limit));
// 	variance = variance / 1000000; // scale down the variance for easier comparison
// 	LOG_DEBUG("[shock_detector.c:%d] %d-%d hz: Mean:%d, Variance:%d, Zeroes:%d\n", __LINE__, lower_limit, upper_limit, (int)mean, variance, zeroes_count);
// 	//END VARIANCE CHECK

// 	//BEGIN VARIANCE CHECK
// 	variance = 0;
// 	sum = 0;
// 	lower_limit = 50; // only check variance for frequencies above this limit
// 	upper_limit = 100; // only check variance for frequencies below this limit
// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		sum += instance->freq_avg->frequency_domain[i].average;
// 		if (instance->freq_avg->frequency_domain[i].average == 0) {
// 			zeroes_count++;
// 		}
// 	}
// 	mean = (float)sum / (upper_limit - lower_limit);

// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		float diff = instance->freq_avg->frequency_domain[i].average - mean;
// 		sq_diff_sum += diff * diff;
// 	}
// 	variance = (int)(sq_diff_sum / (upper_limit - lower_limit));
// 	variance = variance / 1000000; // scale down the variance for easier comparison
// 	LOG_DEBUG("[shock_detector.c:%d] %d-%d hz: Mean:%d, Variance:%d, Zeroes:%d\n", __LINE__, lower_limit, upper_limit, (int)mean, variance, zeroes_count);
// 	//END VARIANCE CHECK

// 	//BEGIN VARIANCE CHECK
// 	variance = 0;
// 	sum = 0;
// 	lower_limit = 100; // only check variance for frequencies above this limit
// 	upper_limit = 500; // only check variance for frequencies below this limit
// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		sum += instance->freq_avg->frequency_domain[i].average;
// 		if (instance->freq_avg->frequency_domain[i].average == 0) {
// 			zeroes_count++;
// 		}
// 	}
// 	mean = (float)sum / (upper_limit - lower_limit);

// 	sq_diff_sum = 0.0;
// 	for (int i = lower_limit; i < upper_limit; i += accuracy) {
// 		float diff = instance->freq_avg->frequency_domain[i].average - mean;
// 		sq_diff_sum += diff * diff;
// 	}
// 	variance = (int)(sq_diff_sum / (upper_limit - lower_limit));
// 	variance = variance / 1000000; // scale down the variance for easier comparison
// 	LOG_DEBUG("[shock_detector.c:%d] %d-%d hz: Mean:%d, Variance:%d, Zeroes:%d\n", __LINE__, lower_limit, upper_limit, (int)mean, variance, zeroes_count);
// 	//END VARIANCE CHECK

// 	// int frequency_point1 = 50;
// 	// int frequency_point2 = 95;
// 	// int magnitude_threshold_low = 44000;
// 	// int magnitude_threshold_high = 25000;

// 	// int lowest_value = INT_MAX;
// 	// int highest_value = INT_MIN;
// 	// int delta = 40000;

// 	// for (int i = 5; i < frequency_point1; i++) {
// 	// 	int* magnitude = &instance->freq_avg->frequency_domain[i].average;
// 	// 	if (*magnitude <= lowest_value) {
// 	// 		lowest_value = *magnitude;
// 	// 	}
// 	// 	if (*magnitude >= highest_value) {
// 	// 		highest_value = *magnitude;
// 	// 	}
// 	// }

// 	// if (highest_value - lowest_value < delta) {
// 	// 	LOG_DEBUG("[shock_detector.c:%d] No shock detected: highest_value - lowest_value = %d < %d\n", __LINE__, highest_value - lowest_value, delta);
// 	// 	return false;
// 	// }

// 	// for (int i = frequency_point1; i < frequency_point2; i++) {
// 	// 	int* magnitude = &instance->freq_avg->frequency_domain[i].average;
// 	// 	if (*magnitude > magnitude_threshold_low) {
// 	// 		LOG_DEBUG("[shock_detector.c:%d] No shock detected: magnitude at frequency %d Hz = %d > %d\n", __LINE__, i, *magnitude, magnitude_threshold_low);
// 	// 		return false;
// 	// 	}
// 	// }
// 	// for (int i = frequency_point2; i < instance->nyquist_domain_size; i++) {
// 	// 	int* magnitude = &instance->freq_avg->frequency_domain[i].average;
// 	// 	if (*magnitude > magnitude_threshold_high) {
// 	// 		LOG_DEBUG("[shock_detector.c:%d] No shock detected: magnitude at frequency %d Hz = %d > %d\n", __LINE__, i, *magnitude, magnitude_threshold_high);
// 	// 		return false;
// 	// 	}
// 	// }
// 	return true;
// }

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
		collect_magnitudes(instance);
		sliding_avg(instance->input, instance->sample_size, AVG_SLIDING_WINDOW, instance->smoothed_input, &instance->smoothed_size);
		process_fft(instance); // process the collected samples with FFT
		postprocess_fft(instance); // post-process the FFT results to find the average over frequency
		//LOG_DEBUG("[shock_detector.c:%d] Frequency ; Magnitude\n", __LINE__);

		// for (int i = 0; i < instance->nyquist_domain_size; i += 3) {
		// 	LOG_DEBUG(" ; %d ; %d\n", i, instance->freq_avg->frequency_domain[i].average);
		// }
		
		// for (int i = 0; i < instance->sample_size; i += 3) {
		// 	LOG_DEBUG(" ; %d ; %d\n", i, (int)(instance->input[i].r) - ACCELEROMETER_EARTH_GRAVITY);
		// }
		

		//if (check_shock(instance)) {
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