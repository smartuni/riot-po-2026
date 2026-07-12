#pragma once

#include <stdint.h>

#include "identity_store.h"
#include "cbor_serialization/common.h"

#define ARRAY_SIZE_ID_PUBID 1           // signed public identity as byte string
#define ARRAY_SIZE_ID_PUBID_SIGNATURE 1 // signed public identity as byte string
#define ARRAY_SIZE_ID_SIGNATURE 1       // signature as byte string
#define ARRAY_SIZE_KID 1
#define ARRAY_SIZE_KEY 1

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
