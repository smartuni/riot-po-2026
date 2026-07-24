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
#pragma once
#ifndef SHOCK_DETECTOR_H
#define SHOCK_DETECTOR_H

#define kiss_fft_scalar int32_t
#define SAMPLE_SIZE 1024
#define SHOCK_THRESHOLD 11 //lower values: more sensitive, higher values: less sensitive
#define FREE_FALL_THRESHOLD 800 //lower values: less sensitive, higher values: more sensitive
#define AVG_SLIDING_WINDOW 5

#include "moving_freq_avg.h"

#include "saul_reg.h"
#include "ztimer.h"
#include "thread.h"
#include "board.h"
#include "phydat.h"
#include "kiss_fft.h"
#include "mutex.h"

#define LOG_LEVEL LOG_DEBUG
#include "log.h"

#include <math.h>
#include <sched.h>
#include <stdlib.h>
#include <semaphore.h>

#define ACCELEROMETER_EARTH_GRAVITY 10500

typedef struct {
	int x;
	int y;
	int z;
} raw_acceleration_t;

typedef int16_t accelerometer_status;
enum {
	NO_SHOCK = 0,
	SHOCK_DETECTED,
	FREE_FALL
};


// Restructures for memory alignment and to avoid padding
typedef struct {
	kernel_pid_t thread_pid;
	mutex_t shock_status_mutex;
	accelerometer_status shock_status;

	saul_reg_t* accel_sensor;
	kiss_fft_cpx input[SAMPLE_SIZE];
	kiss_fft_cpx output[SAMPLE_SIZE];
	kiss_fft_cpx smoothed_input[SAMPLE_SIZE];
	int smoothed_size;
	kiss_fft_cfg cfg;
	moving_freq_avg_t* freq_avg; //rename to frequency domain later

	int sample_size;
	int sampling_period_ms;
	int nyquist_domain_size;
	sem_t shock_count_to_report;
	volatile bool running;
	
	char accel_thread_stack[THREAD_STACKSIZE_DEFAULT];
	raw_acceleration_t raw_accel_data[SAMPLE_SIZE];
} shock_detector_t;

/**
 * @brief Initializes a shock detector instance
 * @param instance Pointer to the shock detector instance
 * @param sampling_period_ms The period in milliseconds between each sample collection
 * @return Pointer to the new shock detector, or NULL if memory allocation failed
 */
int shock_detector_init(shock_detector_t* instance,  int sampling_period_ms);

/**
 * @brief Starts the shock detector
 * @param instance Pointer to the shock detector
 * @return 0 on success, EOVERFLOW on failure
 */
int shock_detector_start(shock_detector_t* instance);

accelerometer_status shock_detector_wait_for_accel_sig(shock_detector_t* instance);


#endif