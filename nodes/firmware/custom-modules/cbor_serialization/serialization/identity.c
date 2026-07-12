
#include <assert.h>
#include <stdint.h>

#include "cbor.h"

#include "cbor_serialization/common.h"
#include "cbor_serialization/identity.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

int _cbor_array_size_identity(size_t *size)
{
    *size = ARRAY_SIZE_KID + ARRAY_SIZE_KEY;

    return 0;
}

int _cbor_array_size_signed_pubid(size_t *size)
{
    *size = ARRAY_SIZE_ID_PUBID + ARRAY_SIZE_ID_PUBID_SIGNATURE;

    return 0;
}

int _cbor_array_size_id_reqres(size_t *size)
{
    _cbor_array_size_signed_pubid(size);
    *size += ARRAY_SIZE_MESSAGE;

    return 0;
}

int _cbor_encode_identity(CborEncoder *encoder, const identity_t *identity)
{
    CborError error;

    // kid
    error = cbor_encode_byte_string(encoder, identity->kid, KID_LEN);
    if (error != CborNoError) {
        return -1;
    }

    // key
    error = cbor_encode_byte_string(encoder, identity->key, ED25519_KEY_LEN);
    if (error != CborNoError) {
        return -1;
    }

    return 0;
}

int _cbor_encode_signed_pubid(CborEncoder *encoder, const signed_identity_t *signed_identity)
{
    CborError error;

    // signed pubid
    error = cbor_encode_byte_string(encoder, signed_identity->cbor_payload, PUBID_LEN);
    if (error != CborNoError) {
        return -1;
    }

    // signature
    error = cbor_encode_byte_string(encoder, signed_identity->signature, PUBID_SIGNATURE_LEN);
    if (error != CborNoError) {
        return -1;
    }

    return 0; 
}

int _cbor_encode_msgtype_id_reqres(CborEncoder *encoder, message_type_t msg_type) {
    CborError error;

    if (msg_type == MESSAGE_ID_REQUEST) {
        error = cbor_encode_simple_value(encoder, MESSAGE_TYPE_ID_REQUEST);
    } else if (msg_type == MESSAGE_ID_RESPONSE) {
        error = cbor_encode_simple_value(encoder, MESSAGE_TYPE_ID_RESPONSE);
    } else {
        DEBUG("_cbor_encode_msgtype_id_reqres: expected message type %d or %d, got %d\n",
              MESSAGE_ID_REQUEST, MESSAGE_ID_RESPONSE, msg_type);
        return -1;
    }
    if (error != CborNoError) {
        return -1;
    }

    return 0;
}


int cbor_serialize_id_reqres(message_type_t msg_type, signed_identity_t *signed_identity,
                             uint8_t *out, size_t *out_len)
{
    assert(out != NULL);
    assert(out_len != NULL);

    CborError error;
    CborEncoder root_encoder;
    cbor_encoder_init(&root_encoder, out, *out_len, 0);

    CborEncoder main_array_encoder;
    size_t main_array_size;
    int res = _cbor_array_size_id_reqres(&main_array_size);
    if (res != 0) {
        DEBUG("cbor_serialize_id_request: error getting main array size\n");
        return -1;
    }

    error = cbor_encoder_create_array(&root_encoder, &main_array_encoder,
                                      main_array_size);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_id_request: error creating main array (%d)\n", error);
        return -1;
    }

    error = cbor_encode_simple_value(&main_array_encoder, ENCODING_V_1);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_id_request: error adding version tag (%d)\n", error);
        return -1;
    }

    res = _cbor_encode_msgtype_id_reqres(&main_array_encoder, msg_type);
    if (res != 0) {
        DEBUG("cbor_serialize_id_request: error encoding msg type\n");
        return -1;
    }

    res = _cbor_encode_signed_pubid(&main_array_encoder, signed_identity);
    if (res != 0) {
        DEBUG("cbor_serialize_id_request: error encoding signed pubid\n");
        return -1;
    }

    error = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_id_request: error closing main array (%d)\n", error);
        return -1;
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, out);
    return 0;
}

int cbor_serialize_signed_public_identity(const signed_identity_t *signed_identity, uint8_t *out, size_t *out_len) {
    assert(out != NULL);
    assert(out_len != NULL);

    CborError error;
    CborEncoder root_encoder;
    cbor_encoder_init(&root_encoder, out, *out_len, 0);

    CborEncoder main_array_encoder;
    size_t main_array_size;
    int res = _cbor_array_size_signed_pubid(&main_array_size);
    if (res != 0) {
        DEBUG("cbor_serialize_signed_public_identity: error getting main array size\n");
        return -1;
    }

    error = cbor_encoder_create_array(&root_encoder, &main_array_encoder,
                                      main_array_size);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_signed_public_identity: error creating main array (%d)\n", error);
        return -1;
    }

    res = _cbor_encode_signed_pubid(&main_array_encoder, signed_identity);
    if (res != 0) {
        DEBUG("cbor_serialize_signed_public_identity: error encoding signed pubid\n");
        return -1;
    }

    error = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_signed_public_identity: error closing main array (%d)\n", error);
        return -1;
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, out);
    return 0;
}

int cbor_serialize_identity(const identity_t *identity, uint8_t *out, size_t *out_len) {
    assert(out != NULL);
    assert(out_len != NULL);

    CborError error;
    CborEncoder root_encoder;
    cbor_encoder_init(&root_encoder, out, *out_len, 0);

    CborEncoder main_array_encoder;
    size_t main_array_size;
    int res = _cbor_array_size_identity(&main_array_size);
    if (res != 0) {
        DEBUG("cbor_serialize_identity: error getting main array size\n");
        return -1;
    }

    error = cbor_encoder_create_array(&root_encoder, &main_array_encoder,
                                      main_array_size);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_identity: error creating main array (%d)\n", error);
        return -1;
    }

    res = _cbor_encode_identity(&main_array_encoder, identity);
    if (res != 0) {
        DEBUG("cbor_serialize_identity: error encoding identity\n");
        return -1;
    }

    error = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_identity: error closing main array (%d)\n", error);
        return -1;
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, out);
    return 0;
}
