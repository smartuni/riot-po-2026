#pragma once

#include <stdint.h>

#include "health_monitor.h"
#include "cbor.h"


#define MESSAGE_TYPE_HEALTH_MONITOR 0x04

// #define RECORD_TYPE_GATE_REPORT 0x00
// #define RECORD_TYPE_GATE_OBSERVATION 0x01
// #define RECORD_TYPE_GATE_COMMAND 0x03
// #define RECORD_TYPE_GATE_JOB 0x04

// #define RECORD_GATE_REPORT_STATE_OPEN 0x00
// #define RECORD_GATE_REPORT_STATE_CLOSED 0x01

// #define ARRAY_SIZE_MESSAGE 2            // version + message type

// #define ARRAY_SIZE_RECORD_HEADER 5      // record type + writer ID + seq + HLC phy. + HLC log.

// #define ARRAY_SIZE_RECORD_SIGNATURE 1   // signature as byte string

// #define ARRAY_SIZE_RECORD_DATA_GATE_REPORT 1
// #define ARRAY_SIZE_RECORD_DATA_GATE_OBSERVATION 2
// #define ARRAY_SIZE_RECORD_DATA_GATE_COMMAND 2
// #define ARRAY_SIZE_RECORD_DATA_GATE_JOB 3

// #define ARRAY_SIZE_ID_PUBID 1           // signed public identity as byte string
// #define ARRAY_SIZE_ID_PUBID_SIGNATURE 1 // signed public identity as byte string
// #define ARRAY_SIZE_ID_SIGNATURE 1       // signature as byte string

// /**
//  * @brief Types of messages.
//  */
// typedef enum {
//     /**< Message containing a single report. */
//     MESSAGE_SINGLE_REPORT = 1,

//     /**< Message requesting an ID. */
//     MESSAGE_ID_REQUEST    = 2,

//     /**< Message responing with an ID. */
//     MESSAGE_ID_RESPONSE   = 3
// } message_type_t;


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