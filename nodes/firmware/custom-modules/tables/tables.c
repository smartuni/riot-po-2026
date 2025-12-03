#include <assert.h>
#include <string.h>

#include "hybrid_logical_clock.h"
#include "crypto_service.h"
#include "store_service.h"

#include "include/tables/keys.h"
#include "include/tables/records.h"
#include "include/tables/types.h"
#include "mutex.h"
#include "tables/records.h"
#include "tables/keys.h"
#include "tables.h"

#include "tables/records.h"

#define LOG_LEVEL LOG_NONE
#include "log.h"

/**
 * @brief Structure passed to the memo list iteration
 */
typedef struct {
    tables_context_t *ctx;          /**< Pointer to the table context */
    const table_record_t *record;   /**< Pointer to the record to evaluate */
} _table_memo_arg_t;

/**
 * @brief Check a single node in the linked list of memos, to be passed to clist_foreach
 *        If a memo matches the query, the callback is called.
 *
 * @param node  The Clist node, part of a table_memo_t structure
 * @param arg   The memo list iteration argument, _table_memo_arg_t structure
 *
 * @retval 0 always
 */
static int _check_and_call_memo(clist_node_t *node, void *arg)
{
    _table_memo_arg_t *memo_arg = (_table_memo_arg_t *)arg;
    table_memo_t *memo = container_of(node, table_memo_t, list);

    if (record_matches_query(memo_arg->record, memo->query) && memo->cb != NULL) {
        memo->cb(memo_arg->ctx, memo_arg->record, memo->query, memo->arg);
    }

    return 0;
}

/**
 * @brief Check across the linked list of memos, whether any of them matches a record
 *
 * @param ctx       Pointer to the table context
 * @param record    Pointer to the record to check
 */
static void _check_and_call_memos(tables_context_t *ctx, const table_record_t *record)
{
    assert(ctx != NULL);
    assert(record != NULL);

    _table_memo_arg_t arg = {
        .ctx = ctx,
        .record = record
    };

    clist_foreach(&ctx->memo_list, _check_and_call_memo, &arg);
}

/**
 * @brief Get the next sequence number for a new record.
 * @param ctx      Pointer to the tables context
 * @param sequence Pointer to store the next sequence number
 */
static void _get_next_sequence(tables_context_t *ctx, record_sequence_t *sequence)
{
    mutex_lock(&ctx->lock);
    ctx->self_sequence++;
    *sequence = ctx->self_sequence;
    mutex_unlock(&ctx->lock);
}

/**
 * @brief Create a new table record with proper header fields set.
 * @param ctx    Pointer to the tables context
 * @param record Pointer to the record to create
 * @param key    Pointer to the key under which to store the record
 */
static int _create_new_record(tables_context_t *ctx, table_record_t *record)
{
    assert(ctx != NULL);
    assert(record != NULL);

    memset(record, 0, sizeof(table_record_t));

    set_record_writer_id(record, ctx->self_id);

    record_sequence_t sequence;
    _get_next_sequence(ctx, &sequence);
    set_record_sequence(record, sequence);

    hlc_timestamp_t timestamp;
    if (hlc_get_current_timestamp(ctx->hlc_ctx, &timestamp) != 0) {
        return -1;
    }
    set_record_timestamp(record, timestamp);

    return 0;
}

int tables_init(tables_context_t *ctx, const node_id_t *self,
                store_service_t *store_service, crypto_service_t *crypto_service,
                hlc_ctx_t *hlc_ctx)
{
    assert(ctx != NULL);
    assert(self != NULL);
    assert(store_service != NULL);
    assert(crypto_service != NULL);
    assert(hlc_ctx != NULL);

    mutex_init(&ctx->lock);

    ctx->self_id = self;
    ctx->self_sequence = 0; // TODO: Load from persistent storage
    ctx->store_service = *store_service;
    ctx->crypto_service = *crypto_service;
    ctx->hlc_ctx = hlc_ctx;
    ctx->memo_list.next = NULL;

    return 0;
}

int tables_put_gate_report(tables_context_t *ctx, gate_state_t state)
{
    table_record_t record;
    table_gate_report_t gate_report;
    table_key_t key;
    int result = 0;

    get_gate_report_key(ctx->self_id, &key);
    result = _create_new_record(ctx, &record);
    if (result != 0) {
        LOG_DEBUG("%s: _create_new_record error\n", __func__);
        return result;
    }

    set_gate_report_state(&gate_report, state);
    set_gate_report_data(&record, &gate_report);

    size_t signature_size;
    /* get signature size */
    result = sign_record(ctx, &record, NULL, &signature_size);
    if (result != 0) {
        LOG_DEBUG("%s: get signature len error\n", __func__);
        return result;
    }

    uint8_t signature_buffer[signature_size];
    /* get signature */
    result = sign_record(ctx, &record, signature_buffer, &signature_size);
    if (result != 0) {
        LOG_DEBUG("%s: sign_record error\n", __func__);
        return result;
    }

    result = put_record_in_store(ctx, &record, &key);
    if (result < 0) {
        LOG_DEBUG("%s: put_record_in_store error\n", __func__);
        return result;
    }

    _check_and_call_memos(ctx, &record);

    return 0;
}

int tables_put_gate_observation(tables_context_t *ctx, const node_id_t *gate_id, gate_state_t state)
{
    table_record_t record;
    table_gate_observation_t gate_observation;
    int result = 0;
    table_key_t key;

    get_gate_observation_key(ctx->self_id, gate_id, &key);
    result = _create_new_record(ctx, &record);
    if (result != 0) {
        return result;
    }

    set_gate_observation_id(&gate_observation, gate_id);
    set_gate_observation_state(&gate_observation, state);
    set_gate_observation_data(&record, &gate_observation);

    size_t signature_size;
    /* get signature size */
    result = sign_record(ctx, &record, NULL, &signature_size);
    if (result != 0) {
        return result;
    }

    uint8_t signature_buffer[signature_size];
    /* get signature */
    result = sign_record(ctx, &record, signature_buffer, &signature_size);
    if (result != 0) {
        return result;
    }

    result = put_record_in_store(ctx, &record, &key);
    if (result < 0) {
        return result;
    }

    _check_and_call_memos(ctx, &record);

    return 0;
}

int tables_put_gate_encounter(tables_context_t *ctx, const node_id_t *gate_id, gate_state_t state,
                              rssi_t rssi)
{
    table_record_t record;
    table_gate_encounter_t gate_encounter;
    int result = 0;
    table_key_t key;

    get_gate_encounter_key(ctx->self_id, gate_id, &key);
    result = _create_new_record(ctx, &record);
    if (result != 0) {
        return result;
    }

    set_gate_encounter_id(&gate_encounter, gate_id);
    set_gate_encounter_state(&gate_encounter, state);
    set_gate_encounter_rssi(&gate_encounter, rssi);
    set_gate_encounter_data(&record, &gate_encounter);

    size_t signature_size;
    /* get signature size */
    result = sign_record(ctx, &record, NULL, &signature_size);
    if (result != 0) {
        return result;
    }

    uint8_t signature_buffer[signature_size];
    /* get signature */
    result = sign_record(ctx, &record, signature_buffer, &signature_size);
    if (result != 0) {
        return result;
    }

    result = put_record_in_store(ctx, &record, &key);
    if (result < 0) {
        return result;
    }

    _check_and_call_memos(ctx, &record);

    return 0;
}

int tables_put_mate_encounter(tables_context_t *ctx, const node_id_t *mate_id, rssi_t rssi)
{
    table_record_t record;
    table_mate_encounter_t mate_encounter;
    int result = 0;
    table_key_t key;

    get_mate_encounter_key(ctx->self_id, mate_id, &key);
    result = _create_new_record(ctx, &record);
    if (result != 0) {
        return result;
    }

    set_mate_encounter_id(&mate_encounter, mate_id);
    set_mate_encounter_rssi(&mate_encounter, rssi);
    set_mate_encounter_data(&record, &mate_encounter);

    size_t signature_size;
    /* get signature size */
    result = sign_record(ctx, &record, NULL, &signature_size);
    if (result != 0) {
        return result;
    }

    uint8_t signature_buffer[signature_size];
    /* get signature */
    result = sign_record(ctx, &record, signature_buffer, &signature_size);
    if (result != 0) {
        return result;
    }

    result = put_record_in_store(ctx, &record, &key);
    if (result < 0) {
        return result;
    }

    _check_and_call_memos(ctx, &record);

    return 0;
}

int tables_merge_record(tables_context_t *ctx, const table_record_t *record,
                        table_merge_result_t *result)
{
    assert(ctx != NULL);
    assert(record != NULL);
    assert(result != NULL);

    int res;
    table_key_t key;
    table_record_type_t type;
    bool merge = false;
    hlc_timestamp_t hlc_incoming;

    LOG_DEBUG("tables_merge_record: merging\n");

    memset(result, 0, sizeof(table_merge_result_t));

    get_record_type(record, &type);
    if (type == RECORD_GATE_ENCOUNTER || type == RECORD_MATE_ENCOUNTER) {
        // these types should not be propagated
        LOG_DEBUG("tables_merge_record: invalid type to merge (%d)\n", type);
        return -1;
    }

    get_record_key(record, &key);
    table_record_t record_in_store;
    get_record_timestamp(record, &hlc_incoming);

    // TODO: Should we only be able to retrieve complete records instead of header?
    res = get_record_header_from_store(ctx, &key, &record_in_store.header);
    if (res != 0) {
        merge = true;
        result->new = true;
        LOG_DEBUG("tables_merge_record: new record, should add it\n");
    }

    // the record exists, we need to decide whether to replace it
    if (!result->new) {
        record_sequence_t seq_in_store, seq_incoming;
        get_record_sequence(record, &seq_incoming);
        get_record_sequence(&record_in_store, &seq_in_store);

        hlc_timestamp_t hlc_in_store;
        get_record_timestamp(&record_in_store, &hlc_in_store);

        if (seq_incoming > seq_in_store) {
            merge = true;
        }
        else if (seq_incoming == seq_in_store) {
            int hlcs = hlc_compare(&hlc_incoming, &hlc_in_store);
            if (hlcs > 0) {
                // incoming is newer
                merge = true;
            }
            else {
                // same time or incoming is older
                merge = false;
            }
        }
        else {
            // incoming is older
            merge = false;
        }
    }

    if (merge) {
        // verify the signature on the record
        res = verify_record(ctx, record);
        if (res != 0) {
            result->rejected_sig = true;
            LOG_DEBUG("tables_merge_record: rejected signature\n");
            return -1;
        }

        if (!result->new) {
            result->updated = true;
            LOG_DEBUG("tables_merge_record: existing record, updating it\n");
        }
        res = hlc_update_with_remote_timestamp(ctx->hlc_ctx, &hlc_incoming, NULL);
        if (res) {
            LOG_DEBUG("hlc_update_with_remote_timestamp failed\n");
            return -1;
        }
        res = put_record_in_store(ctx, record, &key);

        _check_and_call_memos(ctx, record);

        return res;
    }

    LOG_DEBUG("tables_merge_record: record is older, skipping it\n");
    return -1;
}

int tables_iterator_init(tables_context_t *ctx, table_iterator_t *iterator,
                         table_query_t *query)
{
    assert(ctx != NULL);
    assert(iterator != NULL);
    assert(query != NULL);
    assert(iterator->store_iter != NULL);

    iterator->state = ITER_INIT;
    iterator->query = query;

    memset(&iterator->store_query, 0, sizeof(store_service_query_t));
    memset(&iterator->search_key, 0, sizeof(table_key_t));
    memset(&iterator->search_key_mask, 0, sizeof(table_key_t));
    memset(&iterator->record, 0, sizeof(table_record_t));
    memset(&iterator->record_data, 0, sizeof(iterator->record_data));

    return 0;
}

static int _init_store_iterator(tables_context_t *ctx, table_iterator_t *iterator)
{
    assert(ctx != NULL);
    assert(iterator != NULL);
    assert(iterator->query != NULL);

    // TODO: this should probably be in keys.c...
    memset(iterator->search_key, 0, TABLES_KEY_SIZE);
    memset(iterator->search_key_mask, 0, TABLES_KEY_SIZE);

    iterator->store_query.key = (const uint8_t *)&iterator->search_key;
    iterator->store_query.key_mask = (const uint8_t *)&iterator->search_key_mask;
    iterator->store_query.key_len = TABLES_KEY_SIZE;

    if (iterator->query->type != RECORD_UNDEFINED) {
        iterator->search_key[0] = iterator->query->type;
        iterator->search_key_mask[0] = 0xFF;
    }

    make_header_key(&iterator->search_key);
    iterator->search_key_mask[1] = 0xFF;

    if (iterator->query->writer_id != NULL) {
        memcpy(&iterator->search_key[2], iterator->query->writer_id, NODE_ID_SIZE);
        memset(&iterator->search_key_mask[2], 0xFF, NODE_ID_SIZE);
    }

    if (iterator->query->involved_id != NULL) {
        memcpy(&iterator->search_key[2 + NODE_ID_SIZE], iterator->query->involved_id, NODE_ID_SIZE);
        memset(&iterator->search_key_mask[2 + NODE_ID_SIZE], 0xFF, NODE_ID_SIZE);
    }

    int result = store_service_iterator_init(&ctx->store_service, iterator->store_iter,
                                             &iterator->store_query);
    if (result != 0) {
        return -1;
    }

    return 0;
}

int tables_iterator_next(tables_context_t *ctx, table_iterator_t *iterator,
                         table_record_t **record,
                         uint8_t *signature, size_t *signature_len)
{
    assert(ctx != NULL);
    assert(iterator != NULL);
    assert(record != NULL);

    int result;

    LOG_DEBUG("tables_iterator_next: getting next record\n");

    while (1) {
        LOG_DEBUG("tables_iterator_next: state %d\n", iterator->state);
        switch (iterator->state) {
        case ITER_INIT:
            result = _init_store_iterator(ctx, iterator);
            if (result != 0) {
                iterator->state = ITER_ERROR;
                LOG_DEBUG("tables_iterator_next: error initializing store iterator\n");
            }

            iterator->state = ITER_RUN;
            break;

        case ITER_RUN: {
            table_key_t key;
            size_t key_len = sizeof(key);
            size_t header_length = sizeof(iterator->record.header);
            result = store_service_iterator_next(&ctx->store_service, iterator->store_iter,
                                                 key, &key_len,
                                                 (uint8_t *)&(iterator->record.header),
                                                 &header_length);
            if (result != 0) {
                iterator->state = ITER_DONE;
                LOG_DEBUG("tables_iterator_next: no more records\n");
                break;
            }

            size_t _signature_len;
            if (signature_len == NULL) {
                _signature_len = 0;
            }
            else {
                _signature_len = *signature_len;
            }

            result = get_record_from_store(ctx, &key, &iterator->record,
                                           (uint8_t *)&iterator->record_data,
                                           sizeof(iterator->record_data), signature,
                                           _signature_len);
            if (result != 0) {
                iterator->state = ITER_ERROR;
                LOG_DEBUG("tables_iterator_next: error retrieving record from store\n");
                break;
            }

            *record = &iterator->record;

            if (signature_len != NULL) {
                *signature_len = iterator->record.signature_len;
            }

            if (signature != NULL) {
                iterator->record.signature = signature;
            }

            return 0;
        }

        case ITER_ERROR:
        case ITER_DONE:
        case ITER_UNKNOWN:
            return -1;

        }
    }
}


void tables_init_query(table_query_t *query, table_record_type_t type,
                       const node_id_t *writer_id, const node_id_t *involved_id)
{
    assert(query != NULL);

    query->type = type;
    query->writer_id = writer_id;
    query->involved_id = involved_id;
}

void tables_add_memo(tables_context_t *ctx, table_memo_t *memo,
                     const table_query_t *query, table_event_cb_t callback,
                     void *callback_arg)
{
    assert(ctx != NULL);
    assert(memo != NULL);
    assert(query != NULL);
    assert(callback != NULL);

    memo->query = query;
    memo->cb = callback;
    memo->arg = callback_arg;

    clist_rpush(&ctx->memo_list, &memo->list);
}

int tables_remove_memo(tables_context_t *ctx, table_memo_t *memo)
{
    assert(ctx != NULL);
    assert(memo != NULL);

    clist_node_t *removed = clist_remove(&ctx->memo_list, &memo->list);

    if (removed == NULL) {
        return -1;
    }

    return 0;
}
