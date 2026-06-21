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
#ifndef MOVING_FREQ_AVG_H
#define MOVING_FREQ_AVG_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	int average;
	int sum;
	int count;
} element_t;

typedef struct {
	int domain_size;
	element_t* frequency_domain;
} moving_freq_avg_t;

moving_freq_avg_t* moving_freq_avg_new(int domain_size);

/*** @brief Adds a new sample to the moving frequency average.
 ** @param avg       Pointer to the moving frequency average structure
* @param frequency The frequency index for which the sample is being added. Should be in the range [0, domain_size-1].
* @param magnitude The magnitude of the sample to add to the average.
*/
void moving_freq_avg_add_sample(moving_freq_avg_t* avg, int frequency, int magnitude);

/**
* @brief Finalizes the average for all frequencies. Should be called after all samples have been added.
 *
 * @param avg   Pointer to the moving frequency average structure
*/
void moving_freq_avg_finalize(moving_freq_avg_t* avg);
void moving_freq_avg_reset(moving_freq_avg_t* avg);
void moving_freq_avg_delete(moving_freq_avg_t* avg);

#endif