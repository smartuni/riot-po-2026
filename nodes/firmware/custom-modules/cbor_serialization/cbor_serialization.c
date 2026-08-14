#include <assert.h>
#include <stdint.h>

#include "net/loramac.h"
#include "cbor.h"
#include "tables/types.h"
#include "tables/records.h"
#include "identity_store.h"

#include "cbor_serialization.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

extern CborError cbor_deserialize_simple_or_u8(CborValue *value, uint8_t *val);

int _cbor_array_size_record(const table_record_t *record, size_t *size, bool include_sig)
{
    table_record_type_t type;

    get_record_type(record, &type);

    *size = ARRAY_SIZE_MESSAGE + ARRAY_SIZE_RECORD_HEADER;
    if (include_sig) {
        *size += ARRAY_SIZE_RECORD_SIGNATURE;
    }

    if (type == RECORD_GATE_REPORT) {
        *size += ARRAY_SIZE_RECORD_DATA_GATE_REPORT;
    }
    else if (type == RECORD_GATE_OBSERVATION) {
        *size += ARRAY_SIZE_RECORD_DATA_GATE_OBSERVATION;
    }
    else if (type == RECORD_GATE_COMMAND) {
        *size += ARRAY_SIZE_RECORD_DATA_GATE_COMMAND;
    }
    else if (type == RECORD_GATE_JOB) {
        *size += ARRAY_SIZE_RECORD_DATA_GATE_JOB;
    }
    else {
        return -1;
    }

    return 0;
}

int _cbor_encode_hlc_timestamp(const hlc_timestamp_t *hlc, CborEncoder *encoder)
{
    CborError error;

    error = cbor_encode_uint(encoder, hlc->physical);
    if (error != CborNoError) {
        return -1;
    }

    error = cbor_encode_uint(encoder, hlc->logical);
    if (error != CborNoError) {
        return -1;
    }

    return 0;
}

int _cbor_encode_record_header(const table_record_t *record, CborEncoder *encoder)
{
    table_record_type_t type;
    uint8_t type_value;
    CborError error;

    get_record_type(record, &type);

    if (type == RECORD_GATE_REPORT) {
        type_value = RECORD_TYPE_GATE_REPORT;
    }
    else if (type == RECORD_GATE_OBSERVATION) {
        type_value = RECORD_TYPE_GATE_OBSERVATION;
    }
    else if (type == RECORD_GATE_COMMAND) {
        type_value = RECORD_TYPE_GATE_COMMAND;
    }
    else if (type == RECORD_GATE_JOB) {
        type_value = RECORD_TYPE_GATE_JOB;
    }
    else {
        return -1;
    }

    error = cbor_encode_simple_value(encoder, type_value);
    if (error != CborNoError) {
        return -1;
    }

    const node_id_t *writer_id;
    get_record_writer_id(record, &writer_id);
    error = cbor_encode_byte_string(encoder, (const uint8_t *)writer_id, NODE_ID_SIZE);
    if (error != CborNoError) {
        return -1;
    }

    record_sequence_t sequence;
    get_record_sequence(record, &sequence);
    error = cbor_encode_uint(encoder, sequence);
    if (error != CborNoError) {
        return -1;
    }

    hlc_timestamp_t hlc;
    get_record_timestamp(record, &hlc);
    int res = _cbor_encode_hlc_timestamp(&hlc, encoder);
    if (res != 0) {
        return -1;
    }

    return 0;
}

int _cbor_encode_gate_state(CborEncoder *encoder, gate_state_t state)
{
    CborError error;

    if (state == GATE_STATE_OPEN) {
        error = cbor_encode_simple_value(encoder, RECORD_GATE_REPORT_STATE_OPEN);
    }
    else if (state == GATE_STATE_CLOSED) {
        error = cbor_encode_simple_value(encoder, RECORD_GATE_REPORT_STATE_CLOSED);
    }
    else {
        return -1;
    }

    return (error != CborNoError);
}

int _cbor_encode_gate_report(const table_gate_report_t *gate_report,
                             CborEncoder *encoder)
{
    gate_state_t state;

    get_gate_report_state(gate_report, &state);
    if (_cbor_encode_gate_state(encoder, state) != 0) {
        return -1;
    }

    return 0;
}

int _cbor_encode_gate_observation(const table_gate_observation_t *gate_observation,
                                  CborEncoder *encoder)
{
    CborError error;
    const node_id_t *gate_id;
    gate_state_t state;

    get_gate_observation_id(gate_observation, &gate_id);
    get_gate_observation_state(gate_observation, &state);

    error = cbor_encode_byte_string(encoder, (const uint8_t *)gate_id, NODE_ID_SIZE);
    if (error != CborNoError) {
        return -1;
    }

    if (_cbor_encode_gate_state(encoder, state) != 0) {
        return -1;
    }

    return 0;
}

int _cbor_encode_gate_command(const table_gate_command_t *gate_command,
                              CborEncoder *encoder)
{
    CborError error;
    const node_id_t *gate_id;
    gate_state_t state;

    get_gate_command_id(gate_command, &gate_id);
    get_gate_command_state(gate_command, &state);

    error = cbor_encode_byte_string(encoder, (const uint8_t *)gate_id, NODE_ID_SIZE);
    if (error != CborNoError) {
        return -1;
    }

    if (_cbor_encode_gate_state(encoder, state) != 0) {
        return -1;
    }

    return 0;
}

int _cbor_encode_gate_job(const table_gate_job_t *gate_job, CborEncoder *encoder)
{
    CborError error;
    const node_id_t *gate_id;
    const node_id_t *mate_id;
    gate_state_t state;

    get_gate_job_id(gate_job, &gate_id);
    get_gate_job_assigned_device(gate_job, &mate_id);
    get_gate_job_state(gate_job, &state);

    error = cbor_encode_byte_string(encoder, (const uint8_t *)gate_id, NODE_ID_SIZE);
    if (error != CborNoError) {
        return -1;
    }

    error = cbor_encode_byte_string(encoder, (const uint8_t *)mate_id, NODE_ID_SIZE);
    if (error != CborNoError) {
        return -1;
    }

    if (_cbor_encode_gate_state(encoder, state) != 0) {
        return -1;
    }

    return 0;
}

int _cbor_encode_record_signature(const table_record_t *record, CborEncoder *encoder)
{
    CborError error;

    if (record->signature == NULL) {
        DEBUG("_cbor_encode_signature: no signature in record\n");
        return -1;
    }

    error = cbor_encode_byte_string(encoder, record->signature, record->signature_len);
    if (error != CborNoError) {
        return -1;
    }

    return 0;
}

int _cbor_encode_record_data(const table_record_t *record, CborEncoder *encoder)
{
    table_record_type_t type;

    get_record_type(record, &type);

    if (type == RECORD_GATE_REPORT) {
        table_gate_report_t *gate_report;
        get_gate_report_data(record, &gate_report);
        return _cbor_encode_gate_report(gate_report, encoder);
    }
    else if (type == RECORD_GATE_OBSERVATION) {
        table_gate_observation_t *gate_observation;
        get_gate_observation_data(record, &gate_observation);
        return _cbor_encode_gate_observation(gate_observation, encoder);
    }
    else if (type == RECORD_GATE_COMMAND) {
        table_gate_command_t *gate_command;
        get_gate_command_data(record, &gate_command);
        return _cbor_encode_gate_command(gate_command, encoder);
    }
    else if (type == RECORD_GATE_JOB) {
        table_gate_job_t *gate_job;
        get_gate_job_data(record, &gate_job);
        return _cbor_encode_gate_job(gate_job, encoder);
    }
    else {
        return -1;
    }
}

static int _cbor_serialize_record(const table_record_t *record, uint8_t *out, size_t *out_len,
                                  bool include_sig)
{
    assert(record != NULL);
    assert(out != NULL);
    assert(out_len != NULL);

    CborError error;
    CborEncoder root_encoder;
    cbor_encoder_init(&root_encoder, out, *out_len, 0);

    CborEncoder main_array_encoder;
    size_t main_array_size;
    int res = _cbor_array_size_record(record, &main_array_size, include_sig);
    if (res != 0) {
        DEBUG("cbor_serialize_record: error getting main array size\n");
        return -1;
    }

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

    error = cbor_encode_simple_value(&main_array_encoder, MESSAGE_TYPE_SINGLE_REPORT);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error adding message type (%d)\n", error);
        return -1;
    }

    res = _cbor_encode_record_header(record, &main_array_encoder);
    if (res != 0) {
        DEBUG("cbor_serialize_record: error encoding record header\n");
        return -1;
    }

    res = _cbor_encode_record_data(record, &main_array_encoder);
    if (res != 0) {
        DEBUG("cbor_serialize_record: error encoding record data\n");
        return -1;
    }

    if (include_sig) {
        res = _cbor_encode_record_signature(record, &main_array_encoder);
        if (res != 0) {
            DEBUG("cbor_serialize_record: error encoding record signature\n");
            return -1;
        }
    }

    error = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if (error != CborNoError) {
        DEBUG("cbor_serialize_record: error closing main array (%d)\n", error);
        return -1;
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, out);
    return 0;

}

int cbor_serialize_record(const table_record_t *record, uint8_t *out, size_t *out_len)
{
    return _cbor_serialize_record(record, out, out_len, true);
}

int cbor_serialize_record_no_sig(const table_record_t *record, uint8_t *out, size_t *out_len)
{
    return _cbor_serialize_record(record, out, out_len, false);
}

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

int _cbor_decode_version(CborValue *value, uint8_t *version)
{
    assert(value != NULL);
    assert(version != NULL);

    CborError error = cbor_deserialize_simple_or_u8(value, version);
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

    CborError error = cbor_deserialize_simple_or_u8(value, message_type);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for message type (%d)\n", __func__, error);
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

int cbor_deserialize_record(const uint8_t *buffer, size_t buffer_len, table_record_t *record,
                            table_record_data_buffer_t *record_data, uint8_t *signature,
                            size_t *signature_len)
{
    assert(buffer != NULL);
    assert(record != NULL);
    assert(record_data != NULL);
    assert(signature_len != NULL);
    // signature CAN be NULL

    DEBUG("cbor_deserialize_record: decoding buffer of %zu bytes\n", buffer_len);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(buffer, buffer_len, CborValidateStrictMode, &parser,
                             &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_record: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize_record: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_record: error entering main array (%d)\n", error);
        return -1;
    }

    uint8_t version;
    result = _cbor_decode_version(&array_item, &version);
    if (result != 0) {
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_record: error advancing to message type (%d)\n", error);
        return -1;
    }

    uint8_t message_type;
    result = _cbor_decode_message_type(&array_item, &message_type);
    if (result != 0) {
        DEBUG("cbor_deserialize_record: error getting message type\n");
        return -1;
    }

    // For now, we only support single report message type
    if (message_type != MESSAGE_TYPE_SINGLE_REPORT) {
        DEBUG("cbor_deserialize_record: expected message type %d, got %d\n",
              MESSAGE_TYPE_SINGLE_REPORT, message_type);
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_record: error advancing to record (%d)\n", error);
        return -1;
    }

    result = cbor_decode_record(&array_item, record, record_data, signature,
                                signature_len);
    if (result != 0) {
        DEBUG("cbor_deserialize_record: error decoding record\n");
        return -1;
    }

    error = cbor_value_leave_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize_record: error leaving container (%d)\n", error);
        return -1;
    }

    return 0;
}

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
        DEBUG("cbor_deserialize_record: error leaving container (%d)\n", error);
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
