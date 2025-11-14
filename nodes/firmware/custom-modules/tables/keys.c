#include <stddef.h>
#include <string.h>

#include "tables/records.h"
#include "tables/keys.h"

/**
 * @brief Converts a node_id_t to bytes and stores them in the given buffer.
 *
 * Buffer @p buffer must have at least NODE_ID_SIZE bytes available.
 *
 * @param node_id The node ID to convert.
 * @param buffer The buffer to store the bytes.
 * @return A pointer to the position in the buffer after the written bytes.
 */
static uint8_t *_node_id_to_bytes(const node_id_t *node_id, uint8_t *buffer)
{
    memcpy(buffer, *node_id, NODE_ID_SIZE);
    return &buffer[NODE_ID_SIZE];
}

void get_gate_report_key(const node_id_t *gate, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_GATE_REPORT;

    if (gate != NULL) {
        _node_id_to_bytes(gate, &_key[2]);
    }
}

void get_gate_observation_key(const node_id_t *observer, const node_id_t *gate, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_GATE_OBSERVATION;

    if (observer != NULL) {
        _node_id_to_bytes(observer, &_key[2]);
    }

    if (gate != NULL) {
        _node_id_to_bytes(gate, &_key[2 + NODE_ID_SIZE]);
    }
}

void get_gate_encounter_key(const node_id_t *observer, const node_id_t *gate, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_GATE_ENCOUNTER;

    if (observer != NULL) {
        _node_id_to_bytes(observer, &_key[2]);
    }

    if (gate != NULL) {
        _node_id_to_bytes(gate, &_key[2 + NODE_ID_SIZE]);
    }
}

void get_mate_encounter_key(const node_id_t *observer, const node_id_t *mate, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_MATE_ENCOUNTER;

    if (observer != NULL) {
        _node_id_to_bytes(observer, &_key[2]);
    }

    if (mate != NULL) {
        _node_id_to_bytes(mate, &_key[2 + NODE_ID_SIZE]);
    }
}

void get_gate_command_key(const node_id_t *gate, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_GATE_COMMAND;

    if (gate != NULL) {
        _node_id_to_bytes(gate, &_key[2]);
    }
}

void get_gate_job_key(const node_id_t *gate, const node_id_t *device, table_key_t *key)
{
    uint8_t *_key = *key;

    memset(_key, 0, TABLES_KEY_SIZE);

    make_header_key(key);

    _key[0] = RECORD_GATE_JOB;

    if (gate != NULL) {
        _node_id_to_bytes(gate, &_key[2]);
    }

    if (device != NULL) {
        _node_id_to_bytes(device, &_key[2 + NODE_ID_SIZE]);
    }
}

void make_header_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    _key[1] = KEY_TYPE_HEADER;
}

void make_data_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    _key[1] = KEY_TYPE_DATA;
}

void make_signature_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    _key[1] = KEY_TYPE_SIGNATURE;
}

void make_signature_length_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    _key[1] = KEY_TYPE_SIGNATURE_LENGTH;
}

int is_header_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    return _key[1] == KEY_TYPE_HEADER;
}

int is_data_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    return _key[1] == KEY_TYPE_DATA;
}

int is_signature_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    return _key[1] == KEY_TYPE_SIGNATURE;
}

int is_signature_length_key(table_key_t *key)
{
    assert(key != NULL);
    uint8_t *_key = *key;
    return _key[1] == KEY_TYPE_SIGNATURE_LENGTH;
}
