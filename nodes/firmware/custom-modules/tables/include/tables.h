/**
 * @brief       Tables for distributed data storage.
 *
 *
 * @author      Leandro Lanzieri
 */

#pragma once

#include "mutex.h"
#include "hybrid_logical_clock.h"

#include "tables/types.h"

/**
 * @brief Initialize the tables context.
 * @param self       Pointer to ID of this node.
 * @param ctx        Pointer to the tables context to initialize
 * @param store_service Pointer to the store service
 * @param crypto_service Pointer to the crypto service
 * @param hlc_ctx    Pointer to the initialized Hybrid Logical Clock context
 *
 * Pointers provided to this function must remain valid for the lifetime of
 * the tables context.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_init(tables_context_t *ctx, const node_id_t *self,
                store_service_t *store_service, crypto_service_t *crypto_service,
                hlc_ctx_t *hlc_ctx);

/**
 * @{
 * @defgroup Authoritative Writer Functions
 * @brief Functions for authoritative writers to store records in tables.
 *
 * These functions are used to create records where the local node is the authoritative writer.
 */
/**
 * @brief Create and store a gate report record.
 * @param ctx    Pointer to the tables context
 * @param state  State of the gate
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_put_gate_report(tables_context_t *ctx, gate_state_t state);

/**
 * @brief Create and store a gate observation record.
 * @param ctx     Pointer to the tables context
 * @param gate_id ID of the observed gate
 * @param state   State of the gate
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_put_gate_observation(tables_context_t *ctx, const node_id_t *gate_id,
                                gate_state_t state);

/**
 * @brief Create and store a gate encounter record.
 * @param ctx     Pointer to the tables context
 * @param gate_id ID of the encountered gate
 * @param state   State of the gate
 * @param rssi    RSSI of the encounter
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_put_gate_encounter(tables_context_t *ctx, const node_id_t *gate_id,
                              gate_state_t state, rssi_t rssi);

/**
 * @brief Create and store a mate encounter record.
 * @param ctx     Pointer to the tables context
 * @param mate_id ID of the encountered mate
 * @param rssi    RSSI of the encounter
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_put_mate_encounter(tables_context_t *ctx, const node_id_t *mate_id,
                              rssi_t rssi);
/** @} */

/**
 * @brief Attempt to merge a received record into the corresponding table.
 *
 * @param ctx   Pointer to the tables context
 * @param record Pointer to the record to merge
 * @param result Pointer to return the detailed result of the merge operation
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int tables_merge_record(tables_context_t *ctx, const table_record_t *record,
                        table_merge_result_t *result);

/**
 * @brief Initialize a table iterator created with TABLE_ITERATOR
 *
 * @param ctx   Pointer to the tables context
 * @param iterator Pointer to the iterator
 * @param query    Pointer to the initialized table query to execute
 * @param store_iterator Pointer to a store iterator to use
 */
int tables_iterator_init(tables_context_t *ctx, table_iterator_t *iterator,
                         table_query_t *query);

#define TABLE_ITERATOR(name, context)                                                   \
        STORE_ITERATOR(name ## _store_iter, (&(context)->store_service));                     \
        table_iterator_t name;                                                              \
        name.store_iter = name ## _store_iter;

/**
 * @brief Get the next table record from an iterator
 *
 * @param ctx   Pointer to the tables context
 * @param iterator Pointer to the iterator
 * @param record    Out pointer that will point to the next record
 * @param signature Optional pointer to a buffer to store the signature (can be NULL). It
 *                  requires enough space to store the signature.
 * @param signature_len Optional pointer to a size which should contain the length of
 *                      @p signature. If provided, it will be updated with the signature
 *                      length in bytes.
 *
 * @retval 0 when a new record is available
 * @retval negative value when the iterator is exhausted
 *
 * @note The lifetime of the pointer at @p record is tied to the validity of @p iterator.
 */
int tables_iterator_next(tables_context_t *ctx, table_iterator_t *iterator,
                         table_record_t **record, uint8_t *signature,
                         size_t *signature_len);

/**
 * @brief Initialize a table query.
 *
 * @param query     Pointer to the query to initialize
 * @param type      Type of record relevant to the query
 * @param writer_id Writer ID relevant to the query. May be NULL.
 * @param involved_id Involved ID relevant to the query. May be NULL.
 */
void tables_init_query(table_query_t *query, table_record_type_t type,
                       const node_id_t *writer_id, const node_id_t *involved_id);

/**
 * @brief Add a memo to be notified when records that fit a given query have activity
 *
 * @param ctx       Pointer to the table context
 * @param memo      Pointer to the memo object (needs no initialization)
 * @param query     Query to filter the records
 * @param callback  Callback function to call when the query matches
 * @param callback_arg Argument passed to the callback function. May be NULL.
 */
void tables_add_memo(tables_context_t *ctx, table_memo_t *memo,
                     const table_query_t *query, table_event_cb_t callback,
                     void *callback_arg);

/**
 * @brief Remove a memo from the memo list.
 *
 * @param ctx       Pointer to the table context
 * @param memo      Pointer to the memo to remove from the list
 *
 * @retval 0 on success
 * @retval negative value if the memo is not found on the list
 */
int tables_remove_memo(tables_context_t *ctx, table_memo_t *memo);
