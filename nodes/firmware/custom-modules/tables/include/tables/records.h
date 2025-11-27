/**
 * @brief       Record accessor functions for table records.
 *
 *
 * @author      Leandro Lanzieri
 */

#pragma once

#include "tables/types.h"
#include "hybrid_logical_clock.h"

/**
 * @{
 * @defgroup Record Storage and Retrieval
 * @brief Functions to store and retrieve records from the underlying storage.
 */
/**
 * @brief Store a record in the underlying storage.
 * @param ctx       Pointer to the tables context
 * @param record    Pointer to the record to store
 * @param key       Pointer to the key under which to store the record
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int put_record_in_store(tables_context_t *ctx, const table_record_t *record,
                        table_key_t *key);
/** @} */

/**
 * @{
 * @defgroup Record Signing and Verification
 * @brief Functions to sign and verify records using the crypto service.
 */
/**
 * @brief Sign a record using the crypto service.
 *
 * When @p signature_buffer is NULL, the function only populates
 * @p signature_buffer_size with the required size.
 *
 * @param ctx                       Pointer to the tables context
 * @param record                    Pointer to the record to sign
 * @param signature_buffer          Pointer to the buffer to store the signature
 * @param signature_buffer_size     Pointer to the size of the signature buffer; updated
 *                                  with the actual signature size upon success
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int sign_record(tables_context_t *ctx, table_record_t *record,
                uint8_t *signature_buffer, size_t *signature_buffer_size);

/**
 * @brief Verify a record using the table crypto service.
 *
 * @param ctx               Pointer to the tables context
 * @param record            Pointer to the record to verify
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int verify_record(tables_context_t *ctx, const table_record_t *record);
/** @} */

/**
 * @brief Check whether a record matches a given query.
 *
 * @param record            Pointer to the record to check
 * @param query             Pointer to the query to compare to
 *
 * @retval true if the record matches the query
 * @retval false if the record does not match the query
 */
bool record_matches_query(const table_record_t *record, const table_query_t *query);

/**
 * @brief Get a key corresponding to the record.
 *
 * @param record            Pointer to the tables context
 * @param key               Pointer to where the key is placed
 */
void get_record_key(const table_record_t *record, table_key_t *key);

/**
 * @brief Get the size of the data of a record.
 *
 * @param record            Pointer to an initialized record
 */
size_t get_record_data_size(const table_record_t *record);

/**
 * @brief Get a record from store.
 *
 * @param ctx               Pointer to the tables context
 * @param key               Pointer to the key of the record
 * @param record            Pointer to the record to populate
 * @param data_out          Buffer to store the data. For size check
 *                          @ref get_record_data_size()
 * @param data_out_len      Length of @p data_out
 * @param signature_out     Buffer to store the signature. For size check
 *                          @ref get_signature_length_of_record_in_store. If NULL, no
 *                          no signature is fetched.
 * @param signature_out_len Length of @p signature_out
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_record_from_store(tables_context_t *ctx, table_key_t *key,
                          table_record_t *record, uint8_t *data_out,
                          size_t data_out_len, uint8_t *signature_out,
                          size_t signature_out_len);

/**
 * @brief Get the header from a record in store.
 *
 * @param ctx               Pointer to the tables context
 * @param key               Pointer to the key of the record
 * @param header            Pointer to the header to populate
 *
 * @retval 0 if the record exists
 * @retval negative value if error or it does not exist
 */
int get_record_header_from_store(tables_context_t *ctx, table_key_t *key,
                                 table_record_header_t *header);

/**
 * @brief Get the length of the signature of a record that is kept in store.
 *
 * @param ctx               Pointer to the tables context
 * @param key               Pointer to the key of the record to check
 * @param signature_len     Pointer to store the length
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int get_signature_length_of_record_in_store(tables_context_t *ctx, table_key_t *key,
                                            size_t *signature_len);

/**
 * @{
 * @defgroup Record Header Accessors
 * @brief Functions to access record header fields.
 */

/**
 * @brief Get the type of a record.
 * @param record Pointer to the record
 * @param type   Pointer to store the record type
 */
void get_record_type(const table_record_t *record, table_record_type_t *type);

/**
 * @brief Get the writer ID of a record.
 * @param record    Pointer to the record
 * @param writer_id Pointer to store the writer ID
 */
void get_record_writer_id(const table_record_t *record, const node_id_t **writer_id);

/**
 * @brief Set the writer ID of a record.
 * @param record    Pointer to the record
 * @param writer_id Pointer to the writer ID to set
 */
void set_record_writer_id(table_record_t *record, const node_id_t *writer_id);

/**
 * @brief Get the sequence number of a record.
 * @param record   Pointer to the record
 * @param sequence Pointer to store the sequence number
 */
void get_record_sequence(const table_record_t *record, record_sequence_t *sequence);

/**
 * @brief Set the sequence number of a record.
 * @param record   Pointer to the record
 * @param sequence Sequence number to set
 */
void set_record_sequence(table_record_t *record, record_sequence_t sequence);

/**
 * @brief Get the timestamp of a record.
 * @param record    Pointer to the record
 * @param timestamp Pointer to store the timestamp
 */
void get_record_timestamp(const table_record_t *record, hlc_timestamp_t *timestamp);

/**
 * @brief Set the timestamp of a record.
 * @param record    Pointer to the record
 * @param timestamp Timestamp to set
 */
void set_record_timestamp(table_record_t *record, hlc_timestamp_t timestamp);

/**
 * @brief Get the signature of a record.
 * @param record        Pointer to the record
 * @param signature     Pointer to store the signature
 * @param signature_len Pointer to store the signature length
 */
void get_record_signature(const table_record_t *record, uint8_t **signature,
                          size_t *signature_len);

/**
 * @brief Set the signature of a record.
 * @param record        Pointer to the record
 * @param signature     Pointer to the signature to set
 * @param signature_len Length of the signature
 */
void set_record_signature(table_record_t *record, uint8_t *signature,
                          size_t signature_len);
/** @} */

/**
 * @{
 * @defgroup Gate Report Accessors
 * @brief Functions to access gate report fields.
 */

/**
 * @brief Get the data part from a gate report record.
 * @param record Pointer to the record
 * @param data   Pointer to store the gate report data
 *
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_GATE_REPORT or data is NULL
 */
int get_gate_report_data(const table_record_t *record, table_gate_report_t **data);

/**
 * @brief Set the data part in a gate report record.
 * @param record Pointer to the record
 * @param data   Pointer to the gate report data to set
 */
void set_gate_report_data(table_record_t *record, table_gate_report_t *data);

/**
 * @brief Get the state from a gate report.
 * @param data Pointer to the gate report
 * @param state Pointer to store the gate state
 */
void get_gate_report_state(const table_gate_report_t *data, gate_state_t *state);

/**
 * @brief Set the state in a gate report.
 * @param data Pointer to the gate report
 * @param state Gate state to set
 */
void set_gate_report_state(table_gate_report_t *data, gate_state_t state);
/** @} */

/**
 * @{
 * @defgroup Gate Observation Accessors
 * @brief Functions to access gate observation record fields.
 */

/**
 * @brief Get the data part from a gate observation record.
 * @param record Pointer to the record
 * @param data   Pointer to store the gate observation data
 *
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_GATE_OBSERVATION or data is NULL
 */
int get_gate_observation_data(const table_record_t *record,
                              table_gate_observation_t **data);

/**
 * @brief Set the data part in a gate observation record.
 * @param record Pointer to the record
 * @param data   Pointer to the gate observation data to set
 */
void set_gate_observation_data(table_record_t *record, table_gate_observation_t *data);

/**
 * @brief Get the gate ID from a gate observation.
 * @param data  Pointer to the observation
 * @param gate_id Pointer to store the gate ID
 */
void get_gate_observation_id(const table_gate_observation_t *data,
                             const node_id_t **gate_id);

/**
 * @brief Set the gate ID in a gate observation.
 * @param data  Pointer to the observation
 * @param gate_id Pointer to the gate ID to set
 */
void set_gate_observation_id(table_gate_observation_t *data, const node_id_t *gate_id);

/**
 * @brief Get the state from a gate observation.
 * @param data Pointer to the observation
 * @param state  Pointer to store the gate state
 */
void get_gate_observation_state(const table_gate_observation_t *data,
                                gate_state_t *state);

/**
 * @brief Set the state in a gate observation.
 * @param data Pointer to the observation
 * @param state  Gate state to set
 */
void set_gate_observation_state(table_gate_observation_t *data, gate_state_t state);
/** @} */

/**
 * @{
 * @defgroup Gate Encounter Accessors
 * @brief Functions to access gate encounter record fields.
 */

/**
 * @brief Get the data part from a gate encounter record.
 * @param record Pointer to the record
 * @param data   Pointer to store the gate encounter data
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_GATE_ENCOUNTER or data is NULL
 */
int get_gate_encounter_data(const table_record_t *record, table_gate_encounter_t **data);

/**
 * @brief Set the data part in a gate encounter record.
 * @param record Pointer to the record
 * @param data   Pointer to the gate encounter data to set
 */
void set_gate_encounter_data(table_record_t *record, table_gate_encounter_t *data);

/**
 * @brief Get the gate ID from a gate encounter.
 * @param data  Pointer to the encounter
 * @param gate_id Pointer to store the gate ID
 */
void get_gate_encounter_id(const table_gate_encounter_t *data,
                           const node_id_t **gate_id);

/**
 * @brief Set the gate ID in a gate encounter.
 * @param data  Pointer to the encounter
 * @param gate_id Pointer to the gate ID to set
 */
void set_gate_encounter_id(table_gate_encounter_t *data, const node_id_t *gate_id);

/**
 * @brief Get the state from a gate encounter.
 * @param data Pointer to the encounter
 * @param state  Pointer to store the gate state
 */
void get_gate_encounter_state(const table_gate_encounter_t *data, gate_state_t *state);

/**
 * @brief Set the state in a gate encounter.
 * @param data Pointer to the encounter
 * @param state  Gate state to set
 */
void set_gate_encounter_state(table_gate_encounter_t *data, gate_state_t state);

/**
 * @brief Get the RSSI from a gate encounter.
 * @param data Pointer to the encounter
 * @param rssi   Pointer to store the RSSI value
 */
void get_gate_encounter_rssi(const table_gate_encounter_t *data, rssi_t *rssi);

/**
 * @brief Set the RSSI in a gate encounter.
 * @param data Pointer to the encounter
 * @param rssi   RSSI value to set
 */
void set_gate_encounter_rssi(table_gate_encounter_t *data, rssi_t rssi);
/** @} */

/**
 * @{
 * @defgroup Mate Encounter Accessors
 * @brief Functions to access mate encounter record fields.
 */

/**
 * @brief Get the data part from a mate encounter record.
 * @param record Pointer to the record
 * @param data   Pointer to store the mate encounter data
 *
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_MATE_ENCOUNTER or data is NULL
 */
int get_mate_encounter_data(const table_record_t *record, table_mate_encounter_t **data);

/**
 * @brief Set the data part in a mate encounter record.
 * @param record Pointer to the record
 * @param data   Pointer to the mate encounter data to set
 */
void set_mate_encounter_data(table_record_t *record, table_mate_encounter_t *data);

/**
 * @brief Get the mate ID from a mate encounter record.
 * @param data  Pointer to the encounter
 * @param mate_id Pointer to store the mate ID
 */
void get_mate_encounter_id(const table_mate_encounter_t *data, const node_id_t **mate_id);

/**
 * @brief Set the mate ID in a mate encounter record.
 * @param data  Pointer to the encounter
 * @param mate_id Pointer to the mate ID to set
 */
void set_mate_encounter_id(table_mate_encounter_t *data, const node_id_t *mate_id);

/**
 * @brief Get the RSSI from a mate encounter record.
 * @param data Pointer to the encounter
 * @param rssi   Pointer to store the RSSI value
 */
void get_mate_encounter_rssi(const table_mate_encounter_t *data, rssi_t *rssi);

/**
 * @brief Set the RSSI in a mate encounter record.
 * @param data Pointer to the encounter
 * @param rssi   RSSI value to set
 */
void set_mate_encounter_rssi(table_mate_encounter_t *data, rssi_t rssi);
/** @} */

/**
 * @{
 * @defgroup Gate Command Accessors
 * @brief Functions to access gate command record fields.
 */
/**
 * @brief Get the data part from a gate command record.
 * @param record Pointer to the record
 * @param data   Pointer to store the gate command data
 *
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_GATE_COMMAND or data is NULL
 */
int get_gate_command_data(const table_record_t *record, table_gate_command_t **data);

/**
 * @brief Set the data part in a gate command record.
 * @param record Pointer to the record
 * @param data   Pointer to the gate command data to set
 */
void set_gate_command_data(table_record_t *record, table_gate_command_t *data);

/**
 * @brief Get the gate ID from a gate command record.
 * @param record  Pointer to the record
 * @param gate_id Pointer to store the gate ID
 */
void get_gate_command_id(const table_gate_command_t *data, const node_id_t **gate_id);

/**
 * @brief Set the gate ID in a gate command record.
 * @param record  Pointer to the record
 * @param gate_id Pointer to the gate ID to set
 */
void set_gate_command_id(table_gate_command_t *data, const node_id_t *gate_id);

/**
 * @brief Get the state from a gate command record.
 * @param record Pointer to the record
 * @param state  Pointer to store the gate state
 */
void get_gate_command_state(const table_gate_command_t *data, gate_state_t *state);

/**
 * @brief Set the state in a gate command record.
 * @param record Pointer to the record
 * @param state  Gate state to set
 */
void set_gate_command_state(table_gate_command_t *data, gate_state_t state);
/** @} */

/**
 * @{
 * @defgroup Gate Job Accessors
 * @brief Functions to access gate job record fields.
 */
/**
 * @brief Get the data part from a gate job record.
 * @param record Pointer to the record
 * @param data   Pointer to store the gate job data
 *
 * @retval 0 on success
 * @retval -1 if record type is not RECORD_GATE_JOB or data is NULL
 */
int get_gate_job_data(const table_record_t *record, table_gate_job_t **data);

/**
 * @brief Set the data part in a gate job record.
 * @param record Pointer to the record
 * @param data   Pointer to the gate job data to set
 */
void set_gate_job_data(table_record_t *record, table_gate_job_t *data);

/**
 * @brief Get the gate ID from a gate job.
 * @param data  Pointer to the job
 * @param gate_id Pointer to store the gate ID
 */
void get_gate_job_id(const table_gate_job_t *data, const node_id_t **gate_id);

/**
 * @brief Set the gate ID in a gate job.
 * @param data  Pointer to the job
 * @param gate_id Pointer to the gate ID to set
 */
void set_gate_job_id(table_gate_job_t *data, const node_id_t *gate_id);

/**
 * @brief Get the state from a gate job.
 * @param data Pointer to the job
 * @param state  Pointer to store the gate state
 */
void get_gate_job_state(const table_gate_job_t *data, gate_state_t *state);

/**
 * @brief Set the state in a gate job.
 * @param data Pointer to the job
 * @param state  Gate state to set
 */
void set_gate_job_state(table_gate_job_t *data, gate_state_t state);

/**
 * @brief Get the assigned device ID from a gate job.
 * @param data    Pointer to the job
 * @param device_id Pointer to store the device ID
 */
void get_gate_job_assigned_device(const table_gate_job_t *data, const node_id_t **device_id);

/**
 * @brief Set the assigned device ID in a gate job.
 * @param data    Pointer to the job
 * @param device_id Pointer to the device ID to set
 */
void set_gate_job_assigned_device(table_gate_job_t *data, const node_id_t *device_id);

/**
 * @brief Get a string representation from a record type value.
 * @param rt      The record type
 * @return        The string representation of the record type
 */
const char* record_type_tostr(table_record_type_t rt);

/**
 * @brief Get a string representation from a record type value.
 * @param rt      The record type
 * @return        The string representation of the record type
 */
const char* gate_state_tostr(gate_state_t s);
/** @} */
