#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "crypto_service.h"
#include "store_service.h"

#include "tables/keys.h"
#include "tables/records.h"
#include "tables/types.h"

void get_record_type(const table_record_t *record, table_record_type_t *type)
{
    assert(record != NULL);
    assert(type != NULL);

    *type = record->header.type;
}

void get_record_writer_id(const table_record_t *record, const node_id_t **writer_id)
{
    assert(record != NULL);
    assert(writer_id != NULL);

    *writer_id = &record->header.writer;
}

void set_record_writer_id(table_record_t *record, const node_id_t *writer_id)
{
    assert(record != NULL);
    assert(writer_id != NULL);

    memcpy(record->header.writer, writer_id, sizeof(node_id_t));
}

void get_record_sequence(const table_record_t *record, record_sequence_t *sequence)
{
    assert(record != NULL);
    assert(sequence != NULL);

    *sequence = record->header.sequence;
}

void set_record_sequence(table_record_t *record, record_sequence_t sequence)
{
    assert(record != NULL);

    record->header.sequence = sequence;
}

void get_record_timestamp(const table_record_t *record, hlc_timestamp_t *timestamp)
{
    assert(record != NULL);
    assert(timestamp != NULL);

    *timestamp = record->header.timestamp;
}

void set_record_timestamp(table_record_t *record, hlc_timestamp_t timestamp)
{
    assert(record != NULL);

    record->header.timestamp = timestamp;
}

void get_record_signature(const table_record_t *record, uint8_t **signature,
                          size_t *signature_len)
{
    assert(record != NULL);
    assert(signature != NULL);
    assert(signature_len != NULL);

    *signature = record->signature;
    *signature_len = record->signature_len;
}

void set_record_signature(table_record_t *record, uint8_t *signature,
                          size_t signature_len)
{
    assert(record != NULL);

    record->signature = signature;
    record->signature_len = signature_len;
}

void get_record_key(const table_record_t *record, table_key_t *key)
{

    switch (record->header.type) {
    case RECORD_GATE_REPORT:
        get_gate_report_key(&record->header.writer, key);
        break;

    case RECORD_GATE_OBSERVATION:
        get_gate_observation_key(&record->header.writer,
                                 &record->data.gate_observation->gate_id, key);
        break;

    case RECORD_GATE_COMMAND:
        get_gate_command_key(&record->data.gate_command->gate_id, key);
        break;

    case RECORD_GATE_JOB:
        get_gate_job_key(&record->data.gate_job->gate_id,
                         &record->data.gate_job->assigned_device, key);
        break;
    case RECORD_GATE_ENCOUNTER:
        get_gate_encounter_key(&record->header.writer,
                               &record->data.gate_encounter->gate_id, key);
        break;
    case RECORD_MATE_ENCOUNTER:
        get_mate_encounter_key(&record->header.writer,
                               &record->data.mate_encounter->mate_id, key);
        break;

    case RECORD_UNDEFINED:
    default:
        return;
    }
}

size_t get_record_data_size(const table_record_t *record)
{
    switch (record->header.type) {
    case RECORD_GATE_REPORT:
        return sizeof(table_gate_report_t);
    case RECORD_GATE_OBSERVATION:
        return sizeof(table_gate_observation_t);
    case RECORD_GATE_ENCOUNTER:
        return sizeof(table_gate_encounter_t);
    case RECORD_MATE_ENCOUNTER:
        return sizeof(table_mate_encounter_t);
    case RECORD_GATE_COMMAND:
        return sizeof(table_gate_command_t);
    case RECORD_GATE_JOB:
        return sizeof(table_gate_job_t);
    case RECORD_UNDEFINED:
    default:
        return 0;
    }
}

int sign_record(tables_context_t *ctx, table_record_t *record,
                uint8_t *signature_buffer, size_t *signature_buffer_size)
{
    assert(ctx != NULL);
    assert(record != NULL);

    int result = 0;
    size_t header_size = sizeof(table_record_header_t);
    size_t data_size = get_record_data_size(record);
    size_t buffer_size = header_size + data_size;

    alignas(max_align_t) uint8_t buffer[buffer_size];

    memcpy(buffer, &record->header, header_size);
    memcpy(buffer + header_size, record->data.raw, data_size);

    result = crypto_service_sign(&ctx->crypto_service, (const uint8_t *)ctx->self_id,
                                 NODE_ID_SIZE, buffer, buffer_size, signature_buffer,
                                 signature_buffer_size);
    if (result != 0) {
        return result;
    }

    /* If a buffer was passed, populate the signature in the record */
    if (*signature_buffer_size > 0 && signature_buffer != NULL) {
        set_record_signature(record, signature_buffer, *signature_buffer_size);
    }
    return 0;
}

int verify_record(tables_context_t *ctx, const table_record_t *record)
{
    assert(ctx != NULL);
    assert(record != NULL);

    if (record->signature_len <= 0 || record->signature == NULL) {
        return -1;
    }

    size_t header_size = sizeof(table_record_header_t);
    size_t data_size = get_record_data_size(record);
    size_t buffer_size = header_size + data_size;

    alignas(max_align_t) uint8_t buffer[buffer_size];

    memcpy(buffer, &record->header, header_size);
    memcpy(buffer + header_size, record->data.raw, data_size);

    return crypto_service_verify(&ctx->crypto_service,
                                 (const uint8_t *)&record->header.writer,
                                 NODE_ID_SIZE, buffer, buffer_size,
                                 record->signature, record->signature_len);
}

int put_record_in_store(tables_context_t *ctx, const table_record_t *record,
                        table_key_t *key)
{
    assert(ctx != NULL);
    assert(record != NULL);
    assert(key != NULL);

    int result = 0;
    size_t data_size = get_record_data_size(record);
    if (data_size == 0) {
        return -1;
    }

    mutex_lock(&ctx->lock);

    make_header_key(key);
    result = store_service_put(&ctx->store_service, (const uint8_t *)key, TABLES_KEY_SIZE,
                               &record->header, sizeof(table_record_header_t));
    if (result != 0) {
        goto unlock_and_return;
    }

    make_data_key(key);
    result = store_service_put(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, record->data.raw, data_size);
    if (result != 0) {
        goto unlock_and_return;
    }

    make_signature_length_key(key);
    result = store_service_put(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, &record->signature_len,
                               sizeof(record->signature_len));
    if (result != 0) {
        goto unlock_and_return;
    }

    /* Skip signature if not present */
    if (record->signature_len > 0) {
        make_signature_key(key);
        result = store_service_put(&ctx->store_service, (const uint8_t *)key,
                                   TABLES_KEY_SIZE, record->signature,
                                   record->signature_len);
        if (result != 0) {
            goto unlock_and_return;
        }
    }

    result = 0;

unlock_and_return:
    mutex_unlock(&ctx->lock);
    return result;
}

int get_record_from_store(tables_context_t *ctx, table_key_t *key,
                          table_record_t *record, uint8_t *data_out,
                          size_t data_out_len, uint8_t *signature_out,
                          size_t signature_out_len)
{
    assert(ctx != NULL);
    assert(record != NULL);
    assert(key != NULL);
    assert(data_out != NULL);

    int result = 0;
    size_t data_size = get_record_data_size(record);
    if (data_size == 0 || data_size > data_out_len) {
        result = -1;
        goto only_return;
    }

    mutex_lock(&ctx->lock);

    make_header_key(key);
    result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, &record->header,
                               sizeof(table_record_header_t));
    if (result != 0) {
        result = -1;
        goto unlock_and_return;
    }

    make_data_key(key);
    result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, data_out, data_size);
    if (result != 0) {
        result = -1;
        goto unlock_and_return;
    }

    record->data.raw = data_out;

    make_signature_length_key(key);
    result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, &record->signature_len,
                               sizeof(record->signature_len));
    if (result != 0 ||
        (record->signature_len > signature_out_len && signature_out != NULL)) {
        result = -1;
        goto unlock_and_return;
    }

    if (signature_out != NULL && record->signature_len > 0) {
        make_signature_key(key);
        result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                                   TABLES_KEY_SIZE, signature_out,
                                   record->signature_len);
        if (result != 0) {
            result = -1;
            goto unlock_and_return;
        }

        record->signature = signature_out;
    }

unlock_and_return:
    mutex_unlock(&ctx->lock);

only_return:
    return result;
}

int get_record_header_from_store(tables_context_t *ctx, table_key_t *key,
                                 table_record_header_t *header)
{
    assert(ctx != NULL);
    assert(key != NULL);

    int result;

    make_header_key(key);

    mutex_lock(&ctx->lock);

    result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, header, sizeof(table_record_header_t));

    mutex_unlock(&ctx->lock);
    return result;
}

int get_signature_length_of_record_in_store(tables_context_t *ctx, table_key_t *key,
                                            size_t *signature_len)
{
    assert(ctx != NULL);
    assert(key != NULL);
    assert(signature_len != NULL);

    int result;
    table_record_t record;

    make_signature_length_key(key);

    mutex_lock(&ctx->lock);

    result = store_service_get(&ctx->store_service, (const uint8_t *)key,
                               TABLES_KEY_SIZE, &record.signature_len,
                               sizeof(record.signature_len));
    if (result != 0) {
        result = -1;
    }
    else {
        *signature_len = record.signature_len;
    }

    mutex_unlock(&ctx->lock);
    return result;
}

static bool _record_involves_node_id(const table_record_t *record,
                                     const node_id_t *node_id)
{
    assert(record != NULL);
    assert(node_id != NULL);

    // start by checking the writer ID
    const node_id_t *id;
    get_record_writer_id(record, &id);
    if (memcmp(id, node_id, NODE_ID_SIZE) == 0) {
        return true;
    }

    table_record_type_t type;
    get_record_type(record, &type);

    // now, check for each record type
    switch (record->header.type) {
    case RECORD_GATE_OBSERVATION: {
        table_gate_observation_t *observation;
        get_gate_observation_data(record, &observation);
        get_gate_observation_id(observation, &id);
        break;
    }

    case RECORD_GATE_ENCOUNTER: {
        table_gate_encounter_t *encounter;
        get_gate_encounter_data(record, &encounter);
        get_gate_encounter_id(encounter, &id);
        break;
    }

    case RECORD_MATE_ENCOUNTER: {
        table_mate_encounter_t *encounter;
        get_mate_encounter_data(record, &encounter);
        get_mate_encounter_id(encounter, &id);
        break;
    }

    case RECORD_GATE_COMMAND: {
        table_gate_command_t *command;
        get_gate_command_data(record, &command);
        get_gate_command_id(command, &id);
        break;
    }

    case RECORD_GATE_JOB: {
        table_gate_job_t *job;
        get_gate_job_data(record, &job);
        get_gate_job_id(job, &id);

        if (memcmp(id, node_id, NODE_ID_SIZE) == 0) {
            return true;
        }

        get_gate_job_assigned_device(job, &id);
        break;
    }

    case RECORD_GATE_REPORT:
    // Gate reports have only a writer ID, no other associated IDs

    case RECORD_UNDEFINED:
    default:
        return false;
    }

    if (memcmp(id, node_id, NODE_ID_SIZE) == 0) {
        return true;
    }

    return false;
}

bool record_matches_query(const table_record_t *record, const table_query_t *query)
{
    assert(record != NULL);
    assert(query != NULL);

    table_record_type_t type;
    get_record_type(record, &type);

    if (query->type != RECORD_UNDEFINED && type != query->type) {
        return false;
    }

    if (query->writer_id != NULL) {
        const node_id_t *writer_id;
        get_record_writer_id(record, &writer_id);

        if (memcmp(writer_id, query->writer_id, NODE_ID_SIZE) != 0) {
            return false;
        }
    }

    if (query->involved_id != NULL &&
        !_record_involves_node_id(record, query->involved_id)) {
        return false;
    }

    return true;
}
