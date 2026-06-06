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

#include "saul_reg.h"
#include "ztimer.h"
#include "thread.h"
#include "board.h"
#include "phydat.h"

#define LOG_LEVEL   LOG_DEBUG
#include "log.h"
#define LOG_SHOCK_DETECTOR(...) LOG_DEBUG("[shock_detector]: " __VA_ARGS__)

#include <math.h>
#include <sched.h>
#include <stdlib.h>


// Restructures for memory alignment and to avoid padding
typedef struct {
	kernel_pid_t thread_pid;
	saul_reg_t* accel_sensor;
	void (*callback)(void);
	int threshold;
	volatile bool running;
	char accel_thread_stack[THREAD_STACKSIZE_DEFAULT];
} shock_detector_t;


/**
 * @brief Creates a new shock detector to the heap
 * @return Pointer to the new shock detector, or NULL if memory allocation failed
 */
shock_detector_t* shock_detector_new(int threshold);

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