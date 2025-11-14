#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "tables/types.h"

int get_gate_report_data(const table_record_t *record, table_gate_report_t **data)
{
    assert(record != NULL);
    assert(data != NULL);

    if (record->header.type != RECORD_GATE_REPORT || record->data.gate_report == NULL) {
        return -1; // Invalid record type
    }

    *data = record->data.gate_report;
    return 0;
}

void set_gate_report_data(table_record_t *record, table_gate_report_t *data)
{
    assert(record != NULL);
    assert(data != NULL);

    record->header.type = RECORD_GATE_REPORT;
    record->data.gate_report = data;
}

void get_gate_report_state(const table_gate_report_t *data, gate_state_t *state)
{
    assert(data != NULL);
    assert(state != NULL);

    *state = data->state;
}

void set_gate_report_state(table_gate_report_t *data, gate_state_t state)
{
    assert(data != NULL);

    data->state = state;
}
