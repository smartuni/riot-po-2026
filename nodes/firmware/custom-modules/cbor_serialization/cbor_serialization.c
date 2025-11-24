#include <assert.h>
#include <stdint.h>

#include "cbor.h"
#include "tables/types.h"
#include "tables/records.h"

#include "cbor_serialization.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

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

int _cbor_decode_version(CborValue *value, uint8_t *version)
{
    assert(value != NULL);
    assert(version != NULL);

    if (!cbor_value_is_simple_type(value)) {
        DEBUG("_cbor_decode_version: expected simple value for version\n");
        return -1;
    }

    CborError error = cbor_value_get_simple_type(value, version);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_version: error getting encoding version (%d)\n", error);
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

    if (!cbor_value_is_simple_type(value)) {
        DEBUG("_cbor_decode_message_type: expected simple value for message type\n");
        return -1;
    }

    CborError error = cbor_value_get_simple_type(value, message_type);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_message_type: error getting message type (%d)\n", error);
        return -1;
    }

    return 0;
}

int cbor_deserialize(const uint8_t *buffer, size_t buffer_len, table_record_t *record,
                     table_record_data_buffer_t *record_data, uint8_t *signature,
                     size_t *signature_len)
{
    assert(buffer != NULL);
    assert(record != NULL);
    assert(record_data != NULL);
    assert(signature_len != NULL);
    // signature CAN be NULL

    DEBUG("cbor_deserialize: decoding buffer of %zu bytes\n", buffer_len);

    CborParser parser;
    CborValue main_array_iterator;
    CborError error;
    int result;

    error = cbor_parser_init(buffer, buffer_len, CborValidateStrictMode, &parser,
                             &main_array_iterator);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize: error initializing parser (%d)\n", error);
        return -1;
    }

    if (!cbor_value_is_array(&main_array_iterator)) {
        DEBUG("cbor_deserialize: expected main array\n");
        return -1;
    }

    CborValue array_item;
    error = cbor_value_enter_container(&main_array_iterator, &array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize: error entering main array (%d)\n", error);
        return -1;
    }

    uint8_t version;
    result = _cbor_decode_version(&array_item, &version);
    if (result != 0) {
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize: error advancing to message type (%d)\n", error);
        return -1;
    }

    uint8_t message_type;
    result = _cbor_decode_message_type(&array_item, &message_type);
    if (result != 0) {
        DEBUG("cbor_deserialize: error getting message type\n");
        return -1;
    }

    // For now, we only support single report message type
    if (message_type != MESSAGE_TYPE_SINGLE_REPORT) {
        DEBUG("cbor_deserialize: expected message type %d, got %d\n",
              MESSAGE_TYPE_SINGLE_REPORT, message_type);
        return -1;
    }

    error = cbor_value_advance(&array_item);
    if (error != CborNoError) {
        DEBUG("cbor_deserialize: error advancing to record (%d)\n", error);
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
