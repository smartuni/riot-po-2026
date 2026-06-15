#include "include/health_monitor_serialization.h"
#include <assert.h>
#include <stdint.h>

#include "cbor.h"
// #include "tables/types.h"
// #include "tables/records.h"
#include "identity_store.h"

#include "cbor_serialization.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

extern CborError cbor_deserialize_simple_or_u8(CborValue *value, uint8_t *val);


// int _cbor_encode_hlc_timestamp(const hlc_timestamp_t *hlc, CborEncoder *encoder)
// {
//     CborError error;

//     error = cbor_encode_uint(encoder, hlc->physical);
//     if (error != CborNoError) {
//         return -1;
//     }

//     error = cbor_encode_uint(encoder, hlc->logical);
//     if (error != CborNoError) {
//         return -1;
//     }

//     return 0;
// }




// int _cbor_encode_record_signature(const table_record_t *record, CborEncoder *encoder)
// {
//     CborError error;

//     if (record->signature == NULL) {
//         DEBUG("_cbor_encode_signature: no signature in record\n");
//         return -1;
//     }

//     error = cbor_encode_byte_string(encoder, record->signature, record->signature_len);
//     if (error != CborNoError) {
//         return -1;
//     }

//     return 0;
// }





//TODO this is used
int health_monitor_serialize_record_no_sig(const health_monitor_payload_t *record, uint8_t *out, size_t *out_len)
{
    //bool include_sig = false;
    assert(record != NULL);
    assert(out != NULL);
    assert(out_len != NULL);

    CborError error;
    CborEncoder root_encoder;
    cbor_encoder_init(&root_encoder, out, *out_len, 0);

    CborEncoder main_array_encoder;
    size_t main_array_size = sizeof(health_monitor_payload_t);

    error = cbor_encoder_create_array(&root_encoder, &main_array_encoder,
                                      main_array_size);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error creating main array (%d)\n", error);
        return -1;
    }

    error = cbor_encode_simple_value(&main_array_encoder, ENCODING_V_1);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error adding version tag (%d)\n", error);
        return -1;
    }

    error = cbor_encode_simple_value(&main_array_encoder, MESSAGE_TYPE_HEALTH_MONITOR);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error adding message type (%d)\n", error);
        return -1;
    }

    // int res = _cbor_encode_record_header(record, &main_array_encoder);
    // if (res != 0) {
    //     DEBUG("cbor_serialize_record: error encoding record header\n");
    //     return -1;
    // }

    // res = _cbor_encode_record_data(record, &main_array_encoder);
    // if (res != 0) {
    //     DEBUG("cbor_serialize_record: error encoding record data\n");
    //     return -1;
    // }

    // if (include_sig) {
    //     res = _cbor_encode_record_signature(record, &main_array_encoder);
    //     if (res != 0) {
    //         DEBUG("cbor_serialize_record: error encoding record signature\n");
    //         return -1;
    //     }
    // }

    error = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error closing main array (%d)\n", error);
        return -1;
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, out);
    return 0;
}

// static int _cbor_array_size_id_reqres(size_t *size)
// {
//     *size = ARRAY_SIZE_MESSAGE + ARRAY_SIZE_ID_PUBID + ARRAY_SIZE_ID_PUBID_SIGNATURE;

//     return 0;
// }

// static int _cbor_encode_signed_pubid(CborEncoder *encoder, signed_identity_t *signed_identity)
// {
//     CborError error;

//     // signed pubid
//     error = cbor_encode_byte_string(encoder, signed_identity->cbor_payload, PUBID_LEN);
//     if (error != CborNoError) {
//         return -1;
//     }

//     // signature
//     error = cbor_encode_byte_string(encoder, signed_identity->signature, PUBID_SIGNATURE_LEN);
//     if (error != CborNoError) {
//         return -1;
//     }

//     return 0; 
// }

// static int _cbor_encode_msgtype_id_reqres(CborEncoder *encoder, message_type_t msg_type) {
//     CborError error;

//     if (msg_type == MESSAGE_ID_REQUEST) {
//         error = cbor_encode_simple_value(encoder, MESSAGE_TYPE_ID_REQUEST);
//     } else if (msg_type == MESSAGE_ID_RESPONSE) {
//         error = cbor_encode_simple_value(encoder, MESSAGE_TYPE_ID_RESPONSE);
//     } else {
//         DEBUG("_cbor_encode_msgtype_id_reqres: expected message type %d or %d, got %d\n",
//               MESSAGE_ID_REQUEST, MESSAGE_ID_RESPONSE, msg_type);
//         return -1;
//     }
//     if (error != CborNoError) {
//         return -1;
//     }

//     return 0;
// }



// static int _cbor_decode_version(CborValue *value, uint8_t *version)
// {
//     assert(value != NULL);
//     assert(version != NULL);

//     CborError error = cbor_deserialize_simple_or_u8(value, version);
//     if (error != CborNoError) {
//         DEBUG("%s expected simple or unsigned value for version (%d)\n", __func__, error);
//         return -1;
//     }

//     // check supported versions
//     if (*version != ENCODING_V_1) {
//         DEBUG("_cbor_decode_version: expected encoding version %d, got %d\n",
//               ENCODING_V_1, *version);
//         return -1;
//     }

//     return 0;
// }

// static int _cbor_decode_message_type(CborValue *value, uint8_t *message_type)
// {
//     assert(value != NULL);
//     assert(message_type != NULL);

//     CborError error = cbor_deserialize_simple_or_u8(value, message_type);
//     if (error != CborNoError) {
//         DEBUG("%s expected simple or unsigned value for message type (%d)\n", __func__, error);
//         return -1;
//     }

//     return 0;
// }


// static int _cbor_decode_signed_pubid(CborValue *value, signed_identity_t *signed_identity)
// {
//     assert(value != NULL);
//     assert(signed_identity != NULL);

//     CborError error;
//     size_t pubid_len;

//     if (!cbor_value_is_byte_string(value)) {
//         DEBUG("_cbor_decode_signed_pubid: expected byte string for signed pubid\n");
//         return -1;
//     }

//     error = cbor_value_get_string_length(value, &pubid_len);
//     if (error != CborNoError) {
//         DEBUG("_cbor_decode_signed_pubid: error getting signed pubid length (%d)\n", error);
//         return -1;
//     }

//     if (pubid_len != PUBID_LEN) {
//         DEBUG("_cbor_decode_signed_pubid: wrong signed pubid length."
//               " Got %zu, expected %d\n", pubid_len, PUBID_LEN);
//         return -1;
//     }

//     error = cbor_value_copy_byte_string(value, signed_identity->cbor_payload, &pubid_len, value);
//     if (error != CborNoError) {
//         DEBUG("_cbor_decode_signed_pubid: error getting signed pubid (%d)\n", error);
//         return -1;
//     }

//     return 0;
// }

// static int _cbor_decode_signed_pubid_signature(CborValue *value, signed_identity_t *signed_identity)
// {
//     assert(value != NULL);
//     assert(signed_identity != NULL);

//     CborError error;
//     size_t pubid_signature_len;

//     if (!cbor_value_is_byte_string(value)) {
//         DEBUG("_cbor_decode_signed_pubid_signature: expected byte string for signed pubid signature\n");
//         return -1;
//     }

//     error = cbor_value_get_string_length(value, &pubid_signature_len);
//     if (error != CborNoError) {
//         DEBUG("_cbor_decode_signed_pubid_signature: error getting signed pubid signature length (%d)\n", error);
//         return -1;
//     }

//     if (pubid_signature_len != PUBID_SIGNATURE_LEN) {
//         DEBUG("_cbor_decode_signed_pubid_signature: wrong signed pubid signature length."
//               " Got %zu, expected %d\n", pubid_signature_len, PUBID_SIGNATURE_LEN);
//         return -1;
//     }

//     error = cbor_value_copy_byte_string(value, signed_identity->signature, &pubid_signature_len, value);
//     if (error != CborNoError) {
//         DEBUG("_cbor_decode_signed_pubid_signature: error getting signed pubid signature (%d)\n", error);
//         return -1;
//     }

//     return 0;
// }
