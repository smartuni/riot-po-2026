#pragma once

#include "health_monitor_payload.h"
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
int health_monitor_serialize_record_no_sig(const health_monitor_payload_t *record, uint8_t *out, size_t *out_len);