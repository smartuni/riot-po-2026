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

#ifndef SHOCK_DETECTOR_H
#define SHOCK_DETECTOR_H


#define kiss_fft_scalar int

#include "saul_reg.h"
#include "ztimer.h"
#include "thread.h"
#include "board.h"
#include "phydat.h"
#include "kiss_fft.h"

#define LOG_LEVEL   LOG_DEBUG
#include "log.h"
#define LOG_SHOCK_DETECTOR(...) LOG_DEBUG("[shock_detector]: " __VA_ARGS__)

#include <math.h>
#include <sched.h>
#include <stdlib.h>

typedef struct {
	int x;
	int y;
	int z;
} raw_acceleration_t;

// Restructures for memory alignment and to avoid padding
typedef struct {
	kernel_pid_t thread_pid;
	saul_reg_t* accel_sensor;
	kiss_fft_cpx* input;
	kiss_fft_cpx* output;
	void (*callback)(void);
	int threshold;
	int sample_size;
	int sampling_period_ms;
	volatile bool running;
	char accel_thread_stack[THREAD_STACKSIZE_DEFAULT];
	int* sample_array;
} shock_detector_t;


/**
 * @brief Creates a new shock detector to the heap
 * @param threshold The magnitude threshold for shock detection in mm/s^2
 * @param sample_size The number of samples to collect for each FFT analysis. Max frequency domain would be sample_size / 2 + 1 bins.
 * @param sampling_period_ms The period in milliseconds between each sample collection
 * @return Pointer to the new shock detector, or NULL if memory allocation failed
 */
shock_detector_t* shock_detector_new(int threshold, int sample_size, int sampling_period_ms);

/**
 * @brief Starts the shock detector
 * @param detector Pointer to the shock detector
 * @return 0 on success, EOVERFLOW on failure
 */
int shock_detector_start(shock_detector_t* detector);

/**
 * @brief Deletes the shock detector
 * @param detector Pointer to the shock detector
 * @return always return 0
 */
int shock_detector_delete(shock_detector_t* detector);

#endif