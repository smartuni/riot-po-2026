#pragma once

#include <stdint.h>

#include "tables/types.h"
#include "cbor.h"

#define ENCODING_V_1 0x01

#define MESSAGE_TYPE_SINGLE_REPORT 0x01

#define RECORD_TYPE_GATE_REPORT 0x00
#define RECORD_TYPE_GATE_OBSERVATION 0x01
#define RECORD_TYPE_GATE_COMMAND 0x03
#define RECORD_TYPE_GATE_JOB 0x04

#define RECORD_GATE_REPORT_STATE_OPEN 0x00
#define RECORD_GATE_REPORT_STATE_CLOSED 0x01

#define ARRAY_SIZE_MESSAGE 2            // version + message type

#define ARRAY_SIZE_RECORD_HEADER 5      // record type + writer ID + seq + HLC phy. + HLC log.

#define ARRAY_SIZE_RECORD_SIGNATURE 1   // signature as byte string

#define ARRAY_SIZE_RECORD_DATA_GATE_REPORT 1
#define ARRAY_SIZE_RECORD_DATA_GATE_OBSERVATION 2
#define ARRAY_SIZE_RECORD_DATA_GATE_COMMAND 2
#define ARRAY_SIZE_RECORD_DATA_GATE_JOB 3

/**
 * @brief Serialize in CBOR a given record as a message of single record type
 *
 * @param record    Pointer to the record to serialize
 * @param out       Pointer to the buffer that holds enough bytes to store the record.
 * @param out_len   Pointer to the size of @p out, it will return with the amount of used
 *                  bytes.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_serialize_record(table_record_t *record, uint8_t *out, size_t *out_len);

/**
 * @brief Deserialize a CBOR message (only records for now)
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param record            Pointer to the record to populate
 * @param record_data       Memory for the record data
 * @param signature         Buffer to place the deserialized signature. Can be NULL.
 * @param signature_len     Pointer to the length of @p signature. Will contain the
 *                          signature length.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_deserialize(const uint8_t *buffer, size_t buffer_len, table_record_t *record,
                     table_record_data_buffer_t *record_data, uint8_t *signature,
                     size_t *signature_len);

/**
 * @brief Decode a CBOR record.
 *
 * @param array_item        CBOR item that is part of the main array
 * @param record            Pointer to the record to populate
 * @param record_data       Memory for the record data
 * @param signature         Buffer to place the record signature. Can be NULL.
 * @param signature_len     Pointer to the length of @p signature. Will contain the
 *                          signature length.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_decode_record(CborValue *array_item, table_record_t *record,
                       table_record_data_buffer_t *record_data, uint8_t *signature,
                       size_t *signature_len);
