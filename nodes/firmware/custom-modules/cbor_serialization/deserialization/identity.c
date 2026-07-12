#include <assert.h>
#include <stdint.h>

#include "cbor.h"
#include "identity_store.h"

#include "cbor_serialization/common.h"
#include "cbor_serialization/identity.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int _cbor_decode_signed_pubid(CborValue *value, signed_identity_t *signed_identity)
{
    assert(value != NULL);
    assert(signed_identity != NULL);

    CborError error;
    size_t pubid_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_signed_pubid: expected byte string for signed pubid\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &pubid_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_signed_pubid: error getting signed pubid length (%d)\n", error);
        return -1;
    }

    if (pubid_len != PUBID_LEN) {
        DEBUG("_cbor_decode_signed_pubid: wrong signed pubid length."
              " Got %zu, expected %d\n", pubid_len, PUBID_LEN);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, signed_identity->cbor_payload, &pubid_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_signed_pubid: error getting signed pubid (%d)\n", error);
        return -1;
    }

    return 0;
}

static int _cbor_decode_signed_pubid_signature(CborValue *value, signed_identity_t *signed_identity)
{
    assert(value != NULL);
    assert(signed_identity != NULL);

    CborError error;
    size_t pubid_signature_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_signed_pubid_signature: expected byte string for signed pubid signature\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &pubid_signature_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_signed_pubid_signature: error getting signed pubid signature length (%d)\n", error);
        return -1;
    }

    if (pubid_signature_len != PUBID_SIGNATURE_LEN) {
        DEBUG("_cbor_decode_signed_pubid_signature: wrong signed pubid signature length."
              " Got %zu, expected %d\n", pubid_signature_len, PUBID_SIGNATURE_LEN);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, signed_identity->signature, &pubid_signature_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_signed_pubid_signature: error getting signed pubid signature (%d)\n", error);
        return -1;
    }

    return 0;
}

int _cbor_decode_kid(CborValue *value, identity_t *identity_out) {
    assert(value != NULL);
    assert(identity_out != NULL);

    CborError error;
    size_t kid_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_kid: expected byte string for key id\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &kid_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_kid: error getting key id length (%d)\n", error);
        return -1;
    }

    if (kid_len != 4) {
        DEBUG("_cbor_decode_kid: wrong key id length."
              " Got %zu, expected %d\n", kid_len, 4);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, identity_out->kid, &kid_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_kid: error getting key id (%d)\n", error);
        return -1;
    }

    return 0;
}

int _cbor_decode_key(CborValue *value, identity_t *identity_out) {
    assert(value != NULL);
    assert(identity_out != NULL);

    CborError error;
    size_t key_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_key: expected byte string for key\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &key_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_key: error getting key length (%d)\n", error);
        return -1;
    }

    if (key_len != 32) {
        DEBUG("_cbor_decode_key: wrong key length."
              " Got %zu, expected %d\n", key_len, 32);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, identity_out->key, &key_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_key: error getting key (%d)\n", error);
        return -1;
    }

    return 0;
}

int _cbor_decode_loramac_keys(CborValue *value, loramac_keys_t *loramac_keys_out) {
    assert(value != NULL);
    assert(loramac_keys_out != NULL);

    CborError error;
    size_t key_len;

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_loramac_keys: expected byte string for key\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &key_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting joineui key length (%d)\n", error);
        return -1;
    }

    if (key_len != LORAMAC_JOINEUI_LEN) {
        DEBUG("_cbor_decode_loramac_keys: wrong joineui key length."
              " Got %zu, expected %d\n", key_len, LORAMAC_JOINEUI_LEN);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, loramac_keys_out->joineui, &key_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting joineui key (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_loramac_keys: expected byte string for deveui key\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &key_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting deveui key length (%d)\n", error);
        return -1;
    }

    if (key_len != LORAMAC_DEVEUI_LEN) {
        DEBUG("_cbor_decode_loramac_keys: wrong deveui key length."
              " Got %zu, expected %d\n", key_len, LORAMAC_DEVEUI_LEN);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, loramac_keys_out->deveui, &key_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting deveui key (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_byte_string(value)) {
        DEBUG("_cbor_decode_loramac_keys: expected byte string for nwkkey\n");
        return -1;
    }

    error = cbor_value_get_string_length(value, &key_len);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting nwkkey length (%d)\n", error);
        return -1;
    }

    if (key_len != LORAMAC_NWKKEY_LEN) {
        DEBUG("_cbor_decode_loramac_keys: wrong nwkkey length."
              " Got %zu, expected %d\n", key_len, LORAMAC_NWKKEY_LEN);
        return -1;
    }

    error = cbor_value_copy_byte_string(value, loramac_keys_out->nwkkey, &key_len, value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_loramac_keys: error getting nwkkey (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_deserialize_id_reqres(const uint8_t *buffer, size_t buffer_len,
                               signed_identity_t *signed_identity, uint8_t *msg_type)
{
    assert(buffer != NULL);
    assert(signed_identity != NULL);

    DEBUG("cbor_deserialize_id_reqres: decoding buffer of %zu bytes\n", buffer_len);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(buffer, buffer_len, CborValidateStrictMode, &parser,
                             &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_id_reqres: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize_id_reqres: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_id_reqres: error entering main array (%d)\n", error);
        return -1;
    }

    uint8_t version;
    result = _cbor_decode_version(&array_item, &version);
    if (result != 0) {
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_id_reqres: error advancing to message type (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_message_type(&array_item, msg_type);
    if (result != 0) {
        DEBUG("cbor_deserialize_id_reqres: error getting message type\n");
        return -1;
    }

    if (*msg_type != MESSAGE_ID_REQUEST && *msg_type != MESSAGE_ID_RESPONSE) {
        DEBUG("cbor_deserialize_id_reqres: expected message type %d or %d, got %d\n",
              MESSAGE_ID_REQUEST, MESSAGE_ID_RESPONSE, *msg_type);
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_id_reqres: error advancing to record (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_signed_pubid(&array_item, signed_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_id_reqres: error decoding signed pubid\n");
        return -1;
    }

    result = _cbor_decode_signed_pubid_signature(&array_item, signed_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_id_reqres: error decoding signed pubid signature\n");
        return -1;
    }

    error = cbor_value_leave_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_id_reqres: error leaving container (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_deserialize_identity(const uint8_t *data, size_t data_size, identity_t *identity_out) {
    assert(data != NULL);
    assert(identity_out != NULL);

    DEBUG("cbor_deserialize_identity: decoding data of %zu bytes\n", data_size);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(data, data_size, CborValidateStrictMode, &parser, &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_identity: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize_identity: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_identity: error entering main array (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_kid(&array_item, identity_out);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding key id\n");
        return -1;
    }

    result = _cbor_decode_key(&array_item, identity_out);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding key\n");
        return -1;
    }

    error = cbor_value_leave_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_identity: error leaving container (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_deserialize_signed_identity(const uint8_t *data, size_t data_size, signed_identity_t *signed_identity_out) {
    assert(data != NULL);
    assert(signed_identity_out != NULL);

    DEBUG("cbor_deserialize_signed_identity: decoding data of %zu bytes\n", data_size);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(data, data_size, CborValidateStrictMode, &parser, &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_signed_identity: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize_signed_identity: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_signed_identity: error entering main array (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_signed_pubid(&array_item, signed_identity_out);
    if (result != 0) {
        DEBUG("cbor_deserialize_id_reqres: error decoding signed pubid\n");
        return -1;
    }

    result = _cbor_decode_signed_pubid_signature(&array_item, signed_identity_out);
    if (result != 0) {
        DEBUG("cbor_deserialize_id_reqres: error decoding signed pubid signature\n");
        return -1;
    }

    error = cbor_value_leave_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_signed_identity: error leaving container (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_deserialize_provisioning_data(const uint8_t *data, size_t data_size, provisioning_data_t *provisioning_data_out) {
    assert(data != NULL);
    assert(provisioning_data_out != NULL);

    DEBUG("cbor_deserialize_provisioning_data: decoding data of %zu bytes\n", data_size);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(data, data_size, CborValidateStrictMode, &parser, &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_provisioning_data: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize_provisioning_data: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_provisioning_data: error entering main array (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_kid(&array_item, &provisioning_data_out->root_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding root key id\n");
        return -1;
    }

    result = _cbor_decode_key(&array_item, &provisioning_data_out->root_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding root key\n");
        return -1;
    }

    result = _cbor_decode_kid(&array_item, &provisioning_data_out->private_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding private key id\n");
        return -1;
    }

    result = _cbor_decode_key(&array_item, &provisioning_data_out->private_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding private key\n");
        return -1;
    }

    result = _cbor_decode_signed_pubid(&array_item, &provisioning_data_out->own_signed_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding own signed identity\n");
        return -1;
    }

    result = _cbor_decode_signed_pubid_signature(&array_item, &provisioning_data_out->own_signed_identity);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding own signed identity signature\n");
        return -1;
    }

    result = _cbor_decode_loramac_keys(&array_item, &provisioning_data_out->loramac_keys);
    if (result != 0) {
        DEBUG("cbor_deserialize_identity: error decoding loramac keys\n");
        return -1;
    }

    error = cbor_value_leave_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_provisioning_data: error leaving container (%d)\n", error);
        return -1;
    }

    return 0;
}
