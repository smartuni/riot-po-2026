#include <assert.h>
#include <stdint.h>

#include "cbor.h"
#include "tables/records.h"

#include "cbor_serialization/common.h"
#include "cbor_serialization/record.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "od.h"

static int _cbor_decode_hlc_timestamp(CborValue *value, hlc_timestamp_t *hlc)
{
    CborError error;

    if (!cbor_value_is_unsigned_integer(value)) {
        DEBUG("_cbor_decode_hlc_timestamp: expected unsigned int for physical HLC\n");
        return -1;
    }

    uint64_t hlc_value;
    error = cbor_value_get_uint64(value, &hlc_value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_hlc_timestamp: error getting physical HLC (%d)\n", error);
        return -1;
    }

    if (hlc_value >= UINT32_MAX) {
        DEBUG("_cbor_decode_hlc_timestamp: physical HLC too large\n");
        return -1;
    }

    hlc->physical = (hlc_physical_t)hlc_value;

    error = cbor_value_advance(value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_hlc_timestamp: error advancing to logical HLC (%d)\n",
              error);
        return -1;
    }

    if (!cbor_value_is_unsigned_integer(value)) {
        DEBUG("_cbor_decode_hlc_timestamp: expected unsigned int for logical HLC\n");
        CborType type = cbor_value_get_type(value);
        DEBUG("_cbor_decode_hlc_timestamp: found instead %d\n", type);
        return -1;
    }

    error = cbor_value_get_uint64(value, &hlc_value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_hlc_timestamp: error getting logical HLC (%d)\n", error);
        return -1;
    }

    if (hlc_value >= UINT32_MAX) {
        DEBUG("_cbor_decode_hlc_timestamp: logical HLC too large\n");
        return -1;
    }

    hlc->logical = (hlc_logical_t)hlc_value;

    return 0;
}

static int _cbor_decode_record_header(CborValue *value, table_record_header_t *header)
{
    CborError error;
    uint8_t simple_value;
    int result;
    memset(header, 0, sizeof(table_record_header_t));

    error = _cbor_deserialize_simple_or_u8(value, &simple_value);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for record type (%d)\n", __func__, error);
        return -1;
    }

    if (simple_value == RECORD_TYPE_GATE_REPORT) {
        header->type = RECORD_GATE_REPORT;
    }
    else if (simple_value == RECORD_TYPE_GATE_OBSERVATION) {
        header->type = RECORD_GATE_OBSERVATION;
    }
    else if (simple_value == RECORD_TYPE_GATE_COMMAND) {
        header->type = RECORD_GATE_COMMAND;
    }
    else if (simple_value == RECORD_TYPE_GATE_JOB) {
        header->type = RECORD_GATE_JOB;
    }
    else {
        DEBUG("_cbor_decode_record_header: got invalid record type (%d)\n",
              simple_value);
        return -1;
    }

    error = cbor_value_advance(value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_header: error advancing to record writer ID (%d)\n",
              error);
        return -1;
    }

    result = _cbor_decode_node_id(value, &header->writer);
    if (result != 0) {
        DEBUG("_cbor_decode_record_header: error decoding writer ID\n");
        return -1;
    }

    if (!cbor_value_is_unsigned_integer(value)) {
        DEBUG("_cbor_decode_record_header: expected unsigned int for sequence\n");
        return -1;
    }

    error = cbor_value_get_uint64(value, &header->sequence);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_header: error getting sequence (%d)\n", error);
        return -1;
    }

    error = cbor_value_advance(value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_header: error advancing to record HLC (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_hlc_timestamp(value, &header->timestamp);
    if (result != 0) {
        return -1;
    }

    return 0;
}

static int _cbor_decode_gate_report(CborValue *array_item, table_gate_report_t *report)
{
    assert(array_item != NULL);
    assert(report != NULL);

    uint8_t gate_state;
    CborError error = _cbor_deserialize_simple_or_u8(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for gate state (%d)\n", __func__, error);
        return -1;
    }

    if (gate_state == RECORD_GATE_REPORT_STATE_OPEN) {
        report->state = GATE_STATE_OPEN;
    }
    else if (gate_state == RECORD_GATE_REPORT_STATE_CLOSED) {
        report->state = GATE_STATE_CLOSED;
    }
    else {
        DEBUG("_cbor_decode_gate_report: unexpected state %d\n", gate_state);
        return -1;
    }

    return 0;
}

static int _cbor_decode_gate_observation(CborValue *array_item,
                                         table_gate_observation_t *observation)
{
    assert(array_item != NULL);
    assert(observation != NULL);

    CborError error;
    int result;

    result = _cbor_decode_node_id(array_item, &observation->gate_id);
    if (result != 0) {
        DEBUG("_cbor_decode_gate_observation: error getting gate ID\n");
        return -1;
    }

    uint8_t gate_state;
    error = _cbor_deserialize_simple_or_u8(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for gate state (%d)\n", __func__, error);
        return -1;
    }

    if (gate_state == RECORD_GATE_REPORT_STATE_OPEN) {
        observation->state = GATE_STATE_OPEN;
    }
    else if (gate_state == RECORD_GATE_REPORT_STATE_CLOSED) {
        observation->state = GATE_STATE_CLOSED;
    }
    else {
        DEBUG("_cbor_decode_gate_observation: unexpected state %d\n", gate_state);
        return -1;
    }

    return 0;
}

static int _cbor_decode_gate_command(CborValue *array_item,
                                     table_gate_command_t *command)
{
    assert(array_item != NULL);
    assert(command != NULL);

    CborError error;
    int result;

    result = _cbor_decode_node_id(array_item, &command->gate_id);
    if (result != 0) {
        DEBUG("_cbor_decode_gate_command: error getting gate ID\n");
        return -1;
    }

    uint8_t gate_state;
    error = _cbor_deserialize_simple_or_u8(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for gate state (%d)\n", __func__, error);
        return -1;
    }

    if (gate_state == RECORD_GATE_REPORT_STATE_OPEN) {
        command->state = GATE_STATE_OPEN;
    }
    else if (gate_state == RECORD_GATE_REPORT_STATE_CLOSED) {
        command->state = GATE_STATE_CLOSED;
    }
    else {
        DEBUG("_cbor_decode_gate_command: unexpected state %d\n", gate_state);
        return -1;
    }

    return 0;
}

static int _cbor_decode_gate_job(CborValue *array_item,
                                 table_gate_job_t *job)
{
    assert(array_item != NULL);
    assert(job != NULL);

    CborError error;
    int result;

    result = _cbor_decode_node_id(array_item, &job->gate_id);
    if (result != 0) {
        DEBUG("_cbor_decode_gate_job: error getting gate ID\n");
        return -1;
    }

    result = _cbor_decode_node_id(array_item, &job->assigned_device);
    if (result != 0) {
        DEBUG("_cbor_decode_gate_job: error getting mate ID\n");
        return -1;
    }

    uint8_t gate_state;
    error = _cbor_deserialize_simple_or_u8(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("%s expected simple or unsigned value for gate state (%d)\n", __func__, error);
        return -1;
    }

    if (gate_state == RECORD_GATE_REPORT_STATE_OPEN) {
        job->state = GATE_STATE_OPEN;
    }
    else if (gate_state == RECORD_GATE_REPORT_STATE_CLOSED) {
        job->state = GATE_STATE_CLOSED;
    }
    else {
        DEBUG("_cbor_decode_gate_job: unexpected state %d\n", gate_state);
        return -1;
    }

    return 0;
}

static int _cbor_decode_record_data(CborValue *array_item, table_record_t *record,
                                    table_record_data_buffer_t *record_data)
{
    assert(array_item != NULL);
    assert(record != NULL);
    assert(record_data != NULL);

    // CborError error;
    int result;

    switch (record->header.type) {
    case RECORD_GATE_REPORT:
        result = _cbor_decode_gate_report(array_item, &record_data->gate_report);
        break;

    case RECORD_GATE_OBSERVATION:
        result = _cbor_decode_gate_observation(array_item, &record_data->gate_observation);
        break;

    case RECORD_GATE_JOB:
        result = _cbor_decode_gate_job(array_item, &record_data->gate_job);
        break;

    case RECORD_GATE_COMMAND:
        result = _cbor_decode_gate_command(array_item, &record_data->gate_command);
        break;

    default:
        DEBUG("_cbor_decode_record_data: unknown record type (%d)\n", record->header.type);
        return -1;
    }

    if (result != 0) {
        DEBUG("_cbor_decode_record_data: error decoding specific data part\n");
        return -1;
    }
    record->data.raw = record_data;
    return 0;
}

static int _cbor_decode_record_signature(CborValue *array_item, table_record_t *record,
                                         uint8_t *signature, size_t *signature_len)
{
    assert(array_item != NULL);
    assert(signature_len != NULL);
    // signature CAN be NULL

    CborError error;

    if (!cbor_value_is_byte_string(array_item)) {
        DEBUG("_cbor_decode_record_signature: expected byte string for signature\n");
        return -1;
    }

    size_t length;
    error = cbor_value_calculate_string_length(array_item, &length);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_signature: error getting signature length (%d)\n",
              error);
        return -1;
    }

    record->signature_len = length;

    if (signature == NULL) {
        *signature_len = length;

        error = cbor_value_advance(array_item);
        if (error != CborNoError) {
            DEBUG("_cbor_decode_record_signature: error advancing to end (%d)\n", error);
            return -1;
        }

        return 0;
    }

    if (*signature_len < length) {
        DEBUG("_cbor_decode_record_signature: not enough space for signature\n"
              " Got %zu bytes, but %zu are required\n", *signature_len, length
              );
        return -1;
    }

    error = cbor_value_copy_byte_string(array_item, signature, signature_len,
                                        array_item);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_signature: error gettting signature (%d)\n", error);
        return -1;
    }

    record->signature = signature;

    return 0;
}

int _cbor_decode_record(CborValue *array_item, table_record_t *record,
                       table_record_data_buffer_t *record_data, uint8_t *signature,
                       size_t *signature_len)
{
    assert(array_item != NULL);
    assert(record != NULL);
    assert(record_data != NULL);
    assert(signature_len != NULL);
    // signature CAN be NULL

    CborError error;
    int result;

    memset(record, 0, sizeof(table_record_t));
    memset(record_data, 0, sizeof(table_record_data_buffer_t));

    result = _cbor_decode_record_header(array_item, &record->header);
    if (result != 0) {
        DEBUG("_cbor_decode_record: error decoding record header\n");
        return -1;
    }

    error = cbor_value_advance(array_item);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record: error advancing to record data (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_record_data(array_item, record, record_data);
    if (result != 0) {
        DEBUG("_cbor_decode_record: error decoding record data\n");
        return -1;
    }

    error = cbor_value_advance(array_item);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record: error advancing to record signature (%d)\n", error);
        return -1;
    }

    /* if the serialized data does not contain a signature, don't try to decode it */
    if (cbor_value_at_end(array_item)) {
        *signature_len = 0;
        /* in case a signature was expected by the caller but there is none,
         * treat this as an error */
        if (signature) {
            return -1;
        }
    } else {
        result = _cbor_decode_record_signature(array_item, record, signature, signature_len);
        if (result != 0) {
            DEBUG("_cbor_decode_record: error decoding record signature\n");
            return -1;
        }
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

    result = _cbor_decode_record(&array_item, record, record_data, signature,
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
