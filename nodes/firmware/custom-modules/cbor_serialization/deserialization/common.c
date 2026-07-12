#include <assert.h>
#include <stdint.h>

#include "cbor.h"
#include "tables/types.h"

#include "cbor_serialization/common.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

CborError _cbor_deserialize_simple_or_u8(CborValue *value, uint8_t *val)
{
    CborError error;
    if (cbor_value_is_simple_type(value)) {
        error = cbor_value_get_simple_type(value, val);
        if (error != CborNoError) {
            DEBUG("%s cbor_value_get_simple_type error: (%d)\n", __func__, error);
            return error;
        }
        return CborNoError;
    } else if (cbor_value_is_unsigned_integer(value)) {
        uint64_t tmp;
        error = cbor_value_get_uint64(value, &tmp);
        if (error != CborNoError) {
            DEBUG("%s cbor_value_get_uint64 (%d)\n", __func__, error);
            return error;
        }
        if (tmp > 255) {
            DEBUG("%s cbor_value_get_uint64 larger than uint8_t!\n", __func__);
            return CborErrorIllegalType;
        }
        *val = (uint8_t)tmp;
        return CborNoError;
    }
    return CborErrorIllegalType;
}

int _cbor_decode_version(CborValue *value, uint8_t *version)
{
    assert(value != NULL);
    assert(version != NULL);

    CborError error = _cbor_deserialize_simple_or_u8(value, version);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for version (%d)\n", __func__, error);
        return -1;
    }

    // check supported versions
    if (*version != ENCODING_V_1) {
        DEBUG("_cbor_decode_version: expected encoding version %d, got %d\n",
              ENCODING_V_1, *version);
        return -1;
    }

    return 0;
}

int _cbor_decode_message_type(CborValue *value, uint8_t *message_type)
{
    assert(value != NULL);
    assert(message_type != NULL);

    CborError error = _cbor_deserialize_simple_or_u8(value, message_type);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for message type (%d)\n", __func__, error);
        return -1;
    }

    return 0;
}

int _cbor_decode_node_id(CborValue *value, node_id_t *node_id)
{
    assert(value != NULL);
    assert(node_id != NULL);

    CborError error;
    size_t id_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_node_id: expected byte string for writer ID\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &id_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_node_id: error getting writer ID length (%d)\n", error);
        return -1;
    }

    if (id_len != NODE_ID_SIZE) {
        DEBUG("_cbor_decode_node_id: wrong writer ID length."
              " Got %zu, expected %d\n", id_len, NODE_ID_SIZE);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, *node_id, &id_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_node_id: error getting writer ID (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_msg_version_info(const uint8_t *buffer, size_t buffer_len, uint8_t *msg_type)
{
    assert(buffer != NULL);

    DEBUG("cbor_msg_version_info: decoding buffer of %zu bytes\n", buffer_len);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(buffer, buffer_len, CborValidateStrictMode, &parser,
                             &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_msg_version_info: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_msg_version_info: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_msg_version_info: error entering main array (%d)\n", error);
        return -1;
    }

    uint8_t version;
    result = _cbor_decode_version(&array_item, &version);
    if (result != 0) {
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_msg_version_info: error advancing to message type (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_message_type(&array_item, msg_type);
    if (result != 0) {
        DEBUG("cbor_msg_version_info: error getting message type\n");
        return -1;
    }

    return 0;
}
