#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "tables/types.h"

int get_mate_encounter_data(const table_record_t *record, table_mate_encounter_t **data)
{
    assert(record != NULL);
    assert(data != NULL);

    if (record->header.type != RECORD_MATE_ENCOUNTER ||
        record->data.mate_encounter == NULL) {
        return -1; // Invalid record type
    }

    *data = record->data.mate_encounter;
    return 0;
}

void set_mate_encounter_data(table_record_t *record, table_mate_encounter_t *data)
{
    assert(record != NULL);
    assert(data != NULL);

    record->header.type = RECORD_MATE_ENCOUNTER;
    record->data.mate_encounter = data;
}

void get_mate_encounter_id(const table_mate_encounter_t *data, const node_id_t **mate_id)
{
    assert(data != NULL);
    assert(mate_id != NULL);

    *mate_id = &data->mate_id;
}

void set_mate_encounter_id(table_mate_encounter_t *data, const node_id_t *mate_id)
{
    assert(data != NULL);
    assert(mate_id != NULL);

    memcpy(data->mate_id, mate_id, sizeof(node_id_t));
}

void get_mate_encounter_rssi(const table_mate_encounter_t *data, rssi_t *rssi)
{
    assert(data != NULL);
    assert(rssi != NULL);

    *rssi = data->rssi;
}

void set_mate_encounter_rssi(table_mate_encounter_t *data, rssi_t rssi)
{
    assert(data != NULL);

    data->rssi = rssi;
}
