#pragma once

#include <stddef.h>

#include "tables/types.h"

/**
 * @brief Types of keys for storing data on the store service from the table module.
 */
typedef enum {
    /** Key type is undefined */
    KEY_TYPE_UNDEFINED          = 0x00,

    /** Header key type */
    KEY_TYPE_HEADER             = 0x01,

    /** Data key type */
    KEY_TYPE_DATA               = 0x02,

    /** Signature key type */
    KEY_TYPE_SIGNATURE          = 0x03,

    /** Signature length type */
    KEY_TYPE_SIGNATURE_LENGTH   = 0x04,
} table_key_type_t;

/**
 * @brief Generates a key for a gate report record.
 *
 * @param gate The ID of the gate making the report. If NULL, that part of the key is 0.
 * @param key Pointer to key to populate.
 */
void get_gate_report_key(const node_id_t *gate, table_key_t *key);

/**
 * @brief Generates a key for a gate observation record.
 *
 * @param observer The ID of the node making the observation. If NULL, that part of the
 *                 key is 0.
 * @param gate The ID of the gate being observed. If NULL, that part of the key is 0.
 * @param key Pointer to key to populate.
 */
void get_gate_observation_key(const node_id_t *observer, const node_id_t *gate, table_key_t *key);

/**
 * @brief Generates a key for a gate encounter record.
 *
 * @param observer The ID of the node making the observation. If NULL, that part of the
 *                 key is 0.
 * @param gate The ID of the gate being encountered. If NULL, that part of the key is 0.
 * @param key Pointer to key to populate.
 */
void get_gate_encounter_key(const node_id_t *observer, const node_id_t *gate, table_key_t *key);

/**
 * @brief Generates a key for a mate encounter record.
 *
 * @param observer The ID of the node making the observation. If NULL, that part of the
 *                 key is 0.
 * @param mate The ID of the mate being encountered. If NULL, that part of the key is 0.
 * @param key Pointer to key to populate.
 */
void get_mate_encounter_key(const node_id_t *observer, const node_id_t *mate, table_key_t *key);

/**
 * @brief Generates a key for a gate command record.
 *
 * @param gate The ID of the gate the command is for. If NULL, that part of the key is 0.
 * @param key Pointer to key to populate.
 */
void get_gate_command_key(const node_id_t *gate, table_key_t *key);

/**
 * @brief Generates a key for a gate job record.
 *
 * @param gate The ID of the gate the job is for. If NULL, that part of the key is 0.
 * @param device The ID of the device the job is assigned to. If NULL, that part of the
 *               key is 0.
 * @param key Pointer to key to populate.
 */
void get_gate_job_key(const node_id_t *gate, const node_id_t *device, table_key_t *key);

/**
 * @brief Makes a key for the header part of a record.
 *
 * @param key Pointer to key to populate.
 */
void make_header_key(table_key_t *key);

/**
 * @brief Checks if a key is for the header part of a record.
 *
 * @param key Pointer to key to check.
 *
 * @retval 1 if the key is for the header part
 * @retval 0 otherwise.
 */
int is_header_key(table_key_t *key);

/**
 * @brief Makes a key for the data part of a record.
 *
 * @param key Pointer to key to populate.
 */
void make_data_key(table_key_t *key);

/**
 * @brief Checks if a key is for the data part of a record.
 *
 * @param key Pointer to key to check.
 *
 * @retval 1 if the key is for the data part
 * @retval 0 otherwise.
 */
int is_data_key(table_key_t *key);

/**
 * @brief Makes a key for the signature part of a record.
 *
 * @param key Pointer to key to populate.
 */
void make_signature_key(table_key_t *key);

/**
 * @brief Checks if a key is for the signature part of a record.
 *
 * @param key Pointer to key to check.
 *
 * @retval 1 if the key is for the signature part
 * @retval 0 otherwise.
 */
int is_signature_key(table_key_t *key);

/**
 * @brief Makes a key for the signature length part of a record.
 *
 * @param key Pointer to key to populate.
 */
void make_signature_length_key(table_key_t *key);

/**
 * @brief Checks if a key is for the signature length part of a record.
 *
 * @param key Pointer to key to check.
 *
 * @retval 1 if the key is for the signature length part
 * @retval 0 otherwise.
 */
int is_signature_length_key(table_key_t *key);
