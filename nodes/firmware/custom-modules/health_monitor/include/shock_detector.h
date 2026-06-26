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

#define kiss_fft_scalar int
#define SAMPLE_SIZE 4096

#include "health_monitor_payload.h"
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

typedef struct {
	int x;
	int y;
	int z;
} raw_acceleration_t;


// Restructures for memory alignment and to avoid padding
typedef struct {
	kernel_pid_t thread_pid;
	mutex_t shock_status_mutex;
	saul_reg_t* accel_sensor;
	kiss_fft_cpx input[SAMPLE_SIZE];
	kiss_fft_cpx output[SAMPLE_SIZE];
	moving_freq_avg_t* freq_avg; //rename to frequency domain later
	void (*callback)(void);
	int threshold;
	int sampling_period_ms;
	int nyquist_domain_size;
	volatile bool running;
	volatile shock_status_t shock_status;
	char accel_thread_stack[THREAD_STACKSIZE_DEFAULT*2];
} shock_detector_t;

/**
 * @brief Creates a new shock detector to the heap
 * @param threshold The magnitude threshold for shock detection in mm/s^2
 * @param sampling_period_ms The period in milliseconds between each sample collection
 * @return Pointer to the new shock detector, or NULL if memory allocation failed
 */
shock_detector_t* shock_detector_new(int threshold, int sampling_period_ms);

/**
 * @brief Starts the shock detector
 * @param detector Pointer to the shock detector
 * @return 0 on success, EOVERFLOW on failure
 */
int shock_detector_start(shock_detector_t* detector);

int shock_detector_fetch_status(shock_detector_t* detector, shock_status_t* status);

int shock_detector_reset_status(shock_detector_t* detector);

/**
 * @brief Deletes the shock detector
 * @param detector Pointer to the shock detector
 * @return always return 0
 */
int shock_detector_delete(shock_detector_t* detector);

#endif