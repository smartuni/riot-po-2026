#include <assert.h>
#include <stdint.h>

#include "cbor.h"
#include "tables/types.h"
#include "tables/records.h"

#include "cbor_serialization.h"

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

static int _cbor_decode_node_id(CborValue *value, node_id_t *node_id)
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

static int _cbor_decode_record_header(CborValue *value, table_record_header_t *header)
{
    CborError error;
    uint8_t simple_value;
    int result;
    memset(header, 0, sizeof(table_record_header_t));

    if (!cbor_value_is_simple_type(value)) {
        DEBUG("_cbor_decode_record_header: expected simple value for record type\n");
        return -1;
    }

    error = cbor_value_get_simple_type(value, &simple_value);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_record_header: error getting record type (%d)\n", error);
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

    CborError error;

    if (!cbor_value_is_simple_type(array_item)) {
        DEBUG("_cbor_decode_gate_report: expected simple type for gate state\n");
        return -1;
    }

    uint8_t gate_state;
    error = cbor_value_get_simple_type(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_gate_report: error getting gate state\n");
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

    if (!cbor_value_is_simple_type(array_item)) {
        DEBUG("_cbor_decode_gate_observation: expected simple type for gate state\n");
        return -1;
    }

    uint8_t gate_state;
    error = cbor_value_get_simple_type(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_gate_observation: error getting gate state\n");
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

    if (!cbor_value_is_simple_type(array_item)) {
        DEBUG("_cbor_decode_gate_command: expected simple type for gate state\n");
        return -1;
    }

    uint8_t gate_state;
    error = cbor_value_get_simple_type(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_gate_command: error getting gate state\n");
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

    if (!cbor_value_is_simple_type(array_item)) {
        DEBUG("_cbor_decode_gate_job: expected simple type for gate state\n");
        return -1;
    }

    uint8_t gate_state;
    error = cbor_value_get_simple_type(array_item, &gate_state);
    if (error != CborNoError) {
        DEBUG("_cbor_decode_gate_job: error getting gate state\n");
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

int cbor_decode_record(CborValue *array_item, table_record_t *record,
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
        DEBUG("cbor_decode_record: error decoding record header\n");
        return -1;
    }

    error = cbor_value_advance(array_item);
    if (error != CborNoError) {
        DEBUG("cbor_decode_record: error advancing to record data (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_record_data(array_item, record, record_data);
    if (result != 0) {
        DEBUG("cbor_decode_record: error decoding record data\n");
        return -1;
    }

    error = cbor_value_advance(array_item);
    if (error != CborNoError) {
        DEBUG("cbor_decode_record: error advancing to record signature (%d)\n", error);
        return -1;
    }

    result = _cbor_decode_record_signature(array_item, record, signature, signature_len);
    if (result != 0) {
        DEBUG("cbor_decode_record: error decoding record signature\n");
        return -1;
    }

    return 0;
}
