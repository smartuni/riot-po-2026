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
 * @brief ID request message
 */
typedef struct {
    /**< Signed public identity of the sender. */
    signed_identity_t sender_signed_identity;

    /**< KID of the public identity to request. */
    uint8_t kid[KID_LEN];
} id_request_t;

/**
 * @brief ID request message
 */
typedef struct {
    /**< Signed public identity of the sender. */
    signed_identity_t sender_signed_identity;

    /**< KID of the requested public identity. */
    uint8_t kid[KID_LEN];

    /**< The requested signed public identity. */
    signed_identity_t requested_signed_identity;
} id_response_t;

/**
 * @brief Deserialize a CBOR message containing an identification request.
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param id_request        Pointer to the id request to populate.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_deserialize_id_request(const uint8_t *buffer, size_t buffer_len, id_request_t *id_request);
/**
 * @brief Deserialize a CBOR message containing an identification response.
 *
 * @param buffer            Buffer containing the CBOR message.
 * @param buffer_len        Length of @p buffer.
 * @param is_response       Pointer to the id response to populate.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_deserialize_id_response(const uint8_t *buffer, size_t buffer_len, id_response_t *id_response);
int cbor_deserialize_identity(const uint8_t *data, size_t data_size, identity_t *identity_out);
int cbor_deserialize_signed_identity(const uint8_t *data, size_t data_size, signed_identity_t *signed_identity_out);
int cbor_deserialize_provisioning_data(const uint8_t *data, size_t data_size, provisioning_data_t *provisioning_data_out);

/**
 * @brief Serialize in CBOR an identification request as a message.
 *
 * @param id_request        The id request to include in the message.
 * @param out               Pointer to the buffer that holds enough bytes to
 *                          store the message.
 * @param out_len           Pointer to the size of @p out, it will return with
 *                          the amount of used bytes.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_serialize_id_request(id_request_t *id_request, uint8_t *out, size_t* out_len);
/**
 * @brief Serialize in CBOR an identification response as a message.
 *
 * @param id_response       The id response to include in the message.
 * @param out               Pointer to the buffer that holds enough bytes to
 *                          store the message.
 * @param out_len           Pointer to the size of @p out, it will return with
 *                          the amount of used bytes.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int cbor_serialize_id_response(id_response_t *id_response, uint8_t *out, size_t* out_len);
int cbor_serialize_signed_public_identity(const signed_identity_t *signed_identity, uint8_t *out, size_t *out_len);
int cbor_serialize_identity(const identity_t *identity, uint8_t *out, size_t *out_len);
