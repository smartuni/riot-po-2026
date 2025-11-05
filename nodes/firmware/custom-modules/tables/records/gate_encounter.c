#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "tables/types.h"

int get_gate_encounter_data(const table_record_t *record, table_gate_encounter_t **data)
{
    assert(record != NULL);
    assert(data != NULL);

    if (record->header.type != RECORD_GATE_ENCOUNTER ||
        record->data.gate_encounter == NULL) {
        return -1; // Invalid record type
    }

    *data = record->data.gate_encounter;
    return 0;
}

void set_gate_encounter_data(table_record_t *record, table_gate_encounter_t *data)
{
    assert(record != NULL);
    assert(data != NULL);

    record->header.type = RECORD_GATE_ENCOUNTER;
    record->data.gate_encounter = data;
}

void get_gate_encounter_id(const table_gate_encounter_t *data, const node_id_t **gate_id)
{
    assert(data != NULL);
    assert(gate_id != NULL);

    *gate_id = &data->gate_id;
}

void set_gate_encounter_id(table_gate_encounter_t *data, const node_id_t *gate_id)
{
    assert(data != NULL);
    assert(gate_id != NULL);

    memcpy(data->gate_id, gate_id, sizeof(node_id_t));
}

void get_gate_encounter_state(const table_gate_encounter_t *data, gate_state_t *state)
{
    assert(data != NULL);
    assert(state != NULL);

    *state = data->state;
}

void set_gate_encounter_state(table_gate_encounter_t *data, gate_state_t state)
{
    assert(data != NULL);

    data->state = state;
}

void get_gate_encounter_rssi(const table_gate_encounter_t *data, rssi_t *rssi)
{
    assert(data != NULL);
    assert(rssi != NULL);

    *rssi = data->rssi;
}

void set_gate_encounter_rssi(table_gate_encounter_t *data, rssi_t rssi)
{
    assert(data != NULL);

    data->rssi = rssi;
}
