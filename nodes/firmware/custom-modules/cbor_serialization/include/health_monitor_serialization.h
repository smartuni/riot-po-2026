/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Health Monitor Serialization for senseGate
 * @author      Maverick Widjaja <Maverick.widjaja@haw-hamburg.de>
 */

#pragma once
#ifndef HEALTH_MONITOR_SERIALIZATION_H
#define HEALTH_MONITOR_SERIALIZATION_H

#include "health_monitor_payload.h"
#include "cbor_serialization.h"
#include "cbor.h"
#include <stdint.h>

#define MESSAGE_TYPE_HEALTH_MONITOR 0x04

/**
 * @brief Same as cbor_serialize_record but leave out signature field
 *
 * @param record    Pointer to the record to serialize
 * @param out       Pointer to the buffer that holds enough bytes to store the record.
 * @param out_len   Pointer to the size of @p out, it will return with the amount of used
 *                  bytes.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int health_monitor_serialize_record_no_sig(const health_monitor_payload_t* record, uint8_t* out, size_t* out_len, bool use_array);

#endif