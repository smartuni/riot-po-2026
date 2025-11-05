#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "tables/types.h"

int get_gate_job_data(const table_record_t *record, table_gate_job_t **data)
{
    assert(record != NULL);
    assert(data != NULL);

    if (record->header.type != RECORD_GATE_JOB || record->data.gate_job == NULL) {
        return -1; // Invalid record type
    }

    *data = record->data.gate_job;
    return 0;
}

void set_gate_job_data(table_record_t *record, table_gate_job_t *data)
{
    assert(record != NULL);
    assert(data != NULL);

    record->header.type = RECORD_GATE_JOB;
    record->data.gate_job = data;
}

void get_gate_job_id(const table_gate_job_t *data, const node_id_t **gate_id)
{
    assert(data != NULL);
    assert(gate_id != NULL);

    *gate_id = &data->gate_id;
}

void set_gate_job_id(table_gate_job_t *data, const node_id_t *gate_id)
{
    assert(data != NULL);
    assert(gate_id != NULL);

    memcpy(data->gate_id, gate_id, sizeof(node_id_t));
}

void get_gate_job_state(const table_gate_job_t *data, gate_state_t *state)
{
    assert(data != NULL);
    assert(state != NULL);

    *state = data->state;
}

void set_gate_job_state(table_gate_job_t *data, gate_state_t state)
{
    assert(data != NULL);

    data->state = state;
}

void get_gate_job_assigned_device(const table_gate_job_t *data, const node_id_t **device_id)
{
    assert(data != NULL);
    assert(device_id != NULL);

    *device_id = &data->assigned_device;
}

void set_gate_job_assigned_device(table_gate_job_t *data, const node_id_t *device_id)
{
    assert(data != NULL);
    assert(device_id != NULL);

    memcpy(data->assigned_device, device_id, sizeof(node_id_t));
}
