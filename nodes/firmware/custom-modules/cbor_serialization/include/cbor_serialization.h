#pragma once

#include <stdint.h>

#include "tables/types.h"
#include "cbor.h"

#define ENCODING_V_1 0x01

#define MESSAGE_TYPE_SINGLE_REPORT 0x01
#define MESSAGE_TYPE_ID_REQUEST 0x02
#define MESSAGE_TYPE_ID_RESPONSE 0x03

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

#define ARRAY_SIZE_ID_PUBID 1           // signed public identity as byte string
#define ARRAY_SIZE_ID_PUBID_SIGNATURE 1 // signed public identity as byte string
#define ARRAY_SIZE_ID_SIGNATURE 1       // signature as byte string
#define ARRAY_SIZE_KID 1
#define ARRAY_SIZE_KEY 1

/**
 * @brief Types of messages.
 */
typedef enum {
    /**< Message containing a single report. */
    MESSAGE_SINGLE_REPORT = 1,

    /**< Message requesting an ID. */
    MESSAGE_ID_REQUEST    = 2,

    /**< Message responding with an ID. */
    MESSAGE_ID_RESPONSE   = 3
} message_type_t;

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
int cbor_serialize_record(const table_record_t *record, uint8_t *out, size_t *out_len);

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
int cbor_serialize_record_no_sig(const table_record_t *record, uint8_t *out, size_t *out_len);

int cbor_serialize_signed_public_identity(const signed_identity_t *signed_identity, uint8_t *out, size_t *out_len);

int cbor_serialize_identity(const identity_t *identity, uint8_t *out, size_t *out_len);

/**
 * @brief Serialize in CBOR an identification request or response as a message.
 *
 * @param msg_type          Whether to create an identification request or response.
 * @param signed_identity   The signed identity to include in the message.
 * @param out               Pointer to the buffer that holds enough bytes to
 *                          store the message.
 * @param out_len           Pointer to the size of @p out, it will return with
 *                          the amount of used bytes.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_serialize_id_reqres(message_type_t msg_type, signed_identity_t *signed_identity, uint8_t *out, size_t *out_len);

int cbor_serialize_signed_public_identity(const signed_identity_t *signed_identity, uint8_t *out, size_t *out_len);

int cbor_serialize_identity(const identity_t *identity, uint8_t *out, size_t *out_len);

/**
 * @brief Get the message version of the CBOR message.
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param msg_type          Buffer to place the message version.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_msg_version_info(const uint8_t *buffer, size_t buffer_len, uint8_t *msg_type);

/**
 * @brief Deserialize a CBOR message containing a record
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param record            Pointer to the record to populate
 * @param record_data       Memory for the record data
 * @param signature         Buffer to place the deserialized signature. Can be NULL.
 *                          If NULL is passed as signature buffer no error will be generated if
 *                          the serialized data carries no signature.
 *                          If a valid buffer is passed but the data contains no signature
 *                          this will be treated as an error.
 * @param signature_len     Pointer to the length of @p signature. Will contain the
 *                          signature length.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_deserialize_record(const uint8_t *buffer, size_t buffer_len, table_record_t *record,
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

/**
 * @brief Deserialize a CBOR message containing an identification request or response.
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param signed_identity   Pointer to the signed identity to populate.
 * @param message_type      Pointer to the message type to populate.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_deserialize_id_reqres(const uint8_t *buffer, size_t buffer_len,
                               signed_identity_t *signed_identity, uint8_t *msg_type);

int cbor_deserialize_identity(const uint8_t *data, size_t data_size, identity_t *identity_out);
int cbor_deserialize_signed_identity(const uint8_t *data, size_t data_size, signed_identity_t *signed_identity_out);
int cbor_deserialize_provisioning_data(const uint8_t *data, size_t data_size, provisioning_data_t *provisioning_data_out);
