// Ensure this header is included only once per translation unit
#pragma once

#include <stdalign.h>
#include <stdint.h>
#include <stddef.h>

#include "clist.h"
#include "mutex.h"
#include "hybrid_logical_clock.h"
#include "crypto_service.h"
#include "store_service.h"

/**
 * @brief Device type value for a Gate node.
 */
#define DEVICE_TYPE_GATE (0)

/**
 * @brief Device type value for SenseMate node.
 */
#define DEVICE_TYPE_SENSEMATE (1)

/**
 * @brief Device type value for a server (backend) instance.
 */
#define DEVICE_TYPE_SERVER (2)

/**
 * @brief Size of node ID in bytes
 */
#define NODE_ID_SIZE 4

/**
 * @brief Size of keys used in tables.
 */
#define TABLES_KEY_SIZE (2 + 2 * NODE_ID_SIZE)

/**
 * @brief Gate state values
 */
typedef enum {
    /*< Gate is open */
    GATE_STATE_OPEN         = 0,

    /*< Gate is closed */
    GATE_STATE_CLOSED       = 1,

    /*< Gate state is unknown */
    GATE_STATE_UNKNOWN      = 2,

    /*< Gate state is irrelevant */
    GATE_STATE_DONT_CARE    = 3
} gate_state_t;

/**
 * @brief Type representing a node ID
 */
typedef uint8_t node_id_t[NODE_ID_SIZE];

/**
 * @brief Type representing the values represented by the node_id_t type
 */
typedef enum {
    NODE_ID_UNUSED,
    NODE_ID_UNUSED,
    NODE_ID_DEVICE_TYPE,
    NODE_ID_DEVICE_ID
} node_id_value_t;

/**
 * @brief RSSI type
 */
typedef int8_t rssi_t;

/**
 * @brief Type representing a record sequence number, unique per writer
 */
typedef uint64_t record_sequence_t;

/**
 * @brief Type representing a record key
 */
typedef uint8_t table_key_t[TABLES_KEY_SIZE];

/**
 * @brief Type of a table context
 */
typedef struct {
    /**< Self ID of the node */
    const node_id_t *self_id;

    /**< Pointer to the Hybrid Logical Clock context */
    hlc_ctx_t *hlc_ctx;

    /**< Last sequence number for records written by this node */
    record_sequence_t self_sequence;

    /**< Table store service */
    store_service_t store_service;

    /**< Table crypto service */
    crypto_service_t crypto_service;

    /**< Mutex to protect access to context */
    mutex_t lock;

    /**< List of notification memos */
    clist_node_t memo_list;
} tables_context_t;

/**
 * @brief Types of records stored in tables.
 */
typedef enum {
    /** The type of the record is not defined */
    RECORD_UNDEFINED        = -1,

    /**< A report that a gate made from itself */
    RECORD_GATE_REPORT      = 0,

    /**< An observation of a gate made by a mate */
    RECORD_GATE_OBSERVATION = 1,

    /**< An encounter of this device with a gate */
    RECORD_GATE_ENCOUNTER   = 2,

    /**< An encounter of this device with a mate */
    RECORD_MATE_ENCOUNTER   = 3,

    /**< A target state of a gate sent by pilot */
    RECORD_GATE_COMMAND     = 4,

    /**< A job to set a gate's state assigned to a specific mate */
    RECORD_GATE_JOB         = 5,
} table_record_type_t;


/**
 * @brief Header of a table record
 */
typedef struct {
    /**< Type of the record */
    table_record_type_t type;

    /**< ID of the node that wrote the record */
    node_id_t writer;

    /**< Sequence number of the record */
    record_sequence_t sequence;

    /**< Timestamp of the record */
    hlc_timestamp_t timestamp;
} table_record_header_t;

/**
 * @brief Data of a gate report record
 */
typedef struct {
    /**< State of the gate */
    gate_state_t state;
} table_gate_report_t;

/**
 * @brief Data of a gate observation record
 */
typedef struct {
    /**< ID of the gate being observed */
    node_id_t gate_id;

    /**< Observed state of the gate */
    gate_state_t state;
} table_gate_observation_t;

/**
 * @brief Data of a gate encounter record
 */
typedef struct {
    /**< ID of the gate being encountered */
    node_id_t gate_id;

    /**< Encountered state of the gate */
    gate_state_t state;

    /**< RSSI value of the encounter */
    rssi_t rssi;
} table_gate_encounter_t;

/**
 * @brief Data of a mate encounter record
 */
typedef struct {
    /**< ID of the mate being encountered */
    node_id_t mate_id;

    /**< RSSI value of the encounter */
    rssi_t rssi;
} table_mate_encounter_t;

/**
 * @brief Data of a gate command record
 */
typedef struct {
    /**< ID of the gate being commanded */
    node_id_t gate_id;

    /**< Target state of the gate */
    gate_state_t state;
} table_gate_command_t;

/**
 * @brief Data of a gate job record
 */
typedef struct {
    /**< ID of the gate for which the job is assigned */
    node_id_t gate_id;

    /**< Target state of the  gate */
    gate_state_t state;

    /**< ID of the device to which the job is assigned */
    node_id_t assigned_device;
} table_gate_job_t;

/**
 * @brief Data of a table record
 */
typedef union {
    /**< Data of a gate report record */
    table_gate_report_t *gate_report;

    /**< Data of a gate observation record */
    table_gate_observation_t *gate_observation;

    /**< Data of a gate encounter record */
    table_gate_encounter_t *gate_encounter;

    /**< Data of a mate encounter record */
    table_mate_encounter_t *mate_encounter;

    /**< Data of a gate command record */
    table_gate_command_t *gate_command;

    /**< Data of a gate job record */
    table_gate_job_t *gate_job;

    /**< Raw data pointer */
    void *raw;
} table_record_data_t;

/**
 * @brief Union to hold any type of record data
 */
typedef union {
    /**< A gate report */
    table_gate_report_t gate_report;

    /**< A gate observation */
    table_gate_observation_t gate_observation;

    /**< A gate encounter */
    table_gate_encounter_t gate_encounter;

    /**< A mate encounter */
    table_mate_encounter_t mate_encounter;

    /**< A gate command */
    table_gate_command_t gate_command;

    /**< A gate job */
    table_gate_job_t gate_job;
} table_record_data_buffer_t;

/**
 * @brief Table record
 */
typedef struct {
    /**< Header of the record */
    table_record_header_t header;

    /**< Data of the record */
    table_record_data_t data;

    /**< Signature of the record */
    uint8_t *signature;

    /**< Length of the signature */
    size_t signature_len;
} table_record_t;

/**
 * @brief Data of a Per Writer Sequence (PWS) record
 */
typedef struct {
    /**< ID of the node that wrote the record */
    node_id_t writer;

    /**< Maximum sequence number seen */
    record_sequence_t max_sequence;
} table_pws_record_t;

/**
 * @brief Detailed result of a merge operation.
 */
typedef struct {
    /**< True if the table of the record was updated */
    bool updated;

    /**< True if this is the first record in the corresponding table and was added */
    bool new;

    /**< True if the record was rejected due to an invalid signature */
    bool rejected_sig;

    /**< True if the record was invalid or malformed */
    bool invalid_record;
} table_merge_result_t;

/**
 * @brief Query of records on the table module. Initialize using tables_init_query
 */
typedef struct {
    /**< type of record */
    table_record_type_t type;

    /**< ID of the record writer, May be NULL. */
    const node_id_t *writer_id;

    /**< ID involved in the record. May be NULL. This also includes writer Id. */
    const node_id_t *involved_id;
} table_query_t;

/**
 * @brief States of the FSM of a table iterator.
 */
typedef enum {
    /**< Unknown state */
    ITER_UNKNOWN,

    /**< Initialization state */
    ITER_INIT,

    /**< Run state */
    ITER_RUN,

    /**< Done state */
    ITER_DONE,

    /**< Error state */
    ITER_ERROR
} table_iterator_state_t;

/**
 *@brief Table iterator context. Instantiate using TABLE_ITERATOR, and initialize using
         tables_iterator_init.
 */
typedef struct {
    /**< FSM state */
    table_iterator_state_t state;

    /**< Table query associated with the iterator */
    table_query_t *query;

    /**< Opaque store service iterator */
    store_service_iterator_t *store_iter;

    /**< Store service query */
    store_service_query_t store_query;

    /**< Memory for the search key pointed by store_query */
    table_key_t search_key;

    /**< Memory for the search key mask pointed by store_query */
    table_key_t search_key_mask;

    /**< Memory for the retrieved record */
    table_record_t record;

    /**< Buffer for data of the retrieved record */
    table_record_data_buffer_t record_data;
} table_iterator_t;

/**
 * @brief Type of callback passed to table memos, to be notified on records activities.
 *
 * @param ctx       Pointer to the table context
 * @param record    Pointer to the record that triggered the memo
 * @param query     Pointer to the query that matched the record
 * @param arg       Pointer to the argument passed when registering the memo
 */
typedef void (*table_event_cb_t)(tables_context_t *ctx, const table_record_t *record,
                                 const table_query_t *query, void *arg);

/**
 * @brief Memo to be notified on record activities in the tables module. Initialize using
 *        tables_add_memo
 */
typedef struct {
    /**< Clist node */
    clist_node_t list;

    /**< Associated table query */
    const table_query_t *query;

    /**< Callback to call when the query matches a record */
    table_event_cb_t cb;

    /**< Opaque argument to pass to the callback. May be NULL. */
    void *arg;
} table_memo_t;
