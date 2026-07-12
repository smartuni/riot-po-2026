#pragma once

#include <stdint.h>

#include "cbor.h"
#include "tables/types.h"

#define ENCODING_V_1 0x01

#define MESSAGE_TYPE_SINGLE_REPORT 0x01
#define MESSAGE_TYPE_ID_REQUEST 0x02
#define MESSAGE_TYPE_ID_RESPONSE 0x03

#define ARRAY_SIZE_MESSAGE 2            // version + message type

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

CborError _cbor_deserialize_simple_or_u8(CborValue *value, uint8_t *val);

/**
 * @brief For internal use. Decode the message/encoding version.
 * 
 * @param value            CBOR value containing the version.
 * @param version          Buffer to place the message/encoding version.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int _cbor_decode_version(CborValue *value, uint8_t *version);

/**
 * @brief For internal use. Decode the message type.
 * 
 * @param value            CBOR value containing the message type.
 * @param type             Buffer to place the message type.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int _cbor_decode_message_type(CborValue *value, uint8_t *message_type);

/**
 * @brief For internal use. Decode the node ID.
 * 
 * @param value            CBOR value containing node ID.
 * @param node_id          Buffer to place the node ID.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int _cbor_decode_node_id(CborValue *value, node_id_t *node_id);

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
