#include "tables.h"
#include <string.h>
#include "embUnit.h"
#include "embUnit/embUnit.h"
#include "fmt.h"
#include "cbor.h"


// Static tables
static target_state_entry target_state_entry_table[MAX_GATE_COUNT];
static is_state_entry is_state_entry_table[MAX_GATE_COUNT];
static seen_status_entry seen_status_entry_table[MAX_GATE_COUNT];
static jobs_entry jobs_entry_table[MAX_GATE_COUNT];

int target_state_table_to_cbor_test(target_state_entry table[], cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 2); // Entry 2

    // [Table Entry]
    for(int i = 0; i < 2; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, table[i].gateID);
        cbor_encode_simple_value(&singleEntryEncoder, table[i].state);
        cbor_encode_int(&singleEntryEncoder, table[i].timestamp);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int target_state_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, target_state_entry_table[i].gateID);
        cbor_encode_simple_value(&singleEntryEncoder, target_state_entry_table[i].state);
        cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].timestamp);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int is_state_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, is_state_entry_table[i].gateID);
        cbor_encode_simple_value(&singleEntryEncoder, is_state_entry_table[i].state);
        cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].gateTime);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int seen_status_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, seen_status_entry_table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].gateTime);
        cbor_encode_simple_value(&singleEntryEncoder, seen_status_entry_table[i].status);
        cbor_encode_simple_value(&singleEntryEncoder, seen_status_entry_table[i].senseMateID);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int jobs_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, jobs_entry_table[i].gateID);
        cbor_encode_simple_value(&singleEntryEncoder, jobs_entry_table[i].done);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int cbor_to_table(cbor_buffer* buffer) {

    CborParser parser;
    CborValue value;
    CborValue wrapperValue;
    CborValue fieldsValue;
    CborValue entryValue;
    uint8_t tableType;

    target_state_entry returnTargetTable[buffer->cbor_size];
    is_state_entry returnIsTable[buffer->cbor_size];
    seen_status_entry returnSeenTable[buffer->cbor_size];
    jobs_entry returnJobsTable[buffer->cbor_size];

    cbor_parser_init(buffer->buffer, buffer->cbor_size, 0, &parser, &value);
    cbor_value_enter_container(&value, &wrapperValue); // [
    cbor_value_get_simple_type(&wrapperValue, &tableType); // get type of table

    cbor_value_enter_container(&wrapperValue, &fieldsValue); // [
    
    uint8_t id, s, sID, d;
    int ts, gt;

    for(int i = 0; i < buffer->cbor_size; i++) {
        cbor_value_enter_container(&fieldsValue, &entryValue); // [
        switch(tableType) {
            case TARGET_STATE_KEY:
                cbor_value_get_simple_type(&fieldsValue, &id);
                cbor_value_get_simple_type(&fieldsValue, &s);
                cbor_value_get_int(&fieldsValue, &ts);
                target_state_entry newTargetEntry = {id, s, ts};
                returnTargetTable[i] = newTargetEntry;
                break;
            case IS_STATE_KEY:
                cbor_value_get_simple_type(&fieldsValue, &id);
                cbor_value_get_simple_type(&fieldsValue, &s);
                cbor_value_get_int(&fieldsValue, &gt);
                is_state_entry newIsEntry = {id, s, gt};
                returnIsTable[i] = newIsEntry;
                break;
            case SEEN_STATUS_KEY:
                cbor_value_get_simple_type(&fieldsValue, &id);
                cbor_value_get_int(&fieldsValue, &gt);
                cbor_value_get_simple_type(&fieldsValue, &s);
                cbor_value_get_simple_type(&fieldsValue, &sID);
                seen_status_entry newSeenEntry = {id, gt, s, sID};
                returnSeenTable[i] = newSeenEntry;
                break;
            case JOBS_KEY:
                cbor_value_get_simple_type(&fieldsValue, &id);
                cbor_value_get_simple_type(&fieldsValue, &d);
                jobs_entry newJobsEntry = {id, d};
                returnJobsTable[i] = newJobsEntry;
                break;
        }   
        cbor_value_leave_container 	(&fieldsValue,&entryValue); // ]	
    }

    cbor_value_leave_container 	(&wrapperValue, &fieldsValue); // ]	
    cbor_value_leave_container 	(&value, &wrapperValue); // ]	
    
    // TODO: Funtionen zum Integrieren aufrufen
    switch(tableType) {
            case TARGET_STATE_KEY:
                (void) returnTargetTable;
                break;
            case IS_STATE_KEY:
                (void) returnIsTable;
                break;
            case SEEN_STATUS_KEY:
                (void) returnSeenTable;
                break;
            case JOBS_KEY:
                (void) returnJobsTable;
                break;
            default:
                return -1;
    }

    return 0;
}

int target_state_table_to_cbor_many(target_state_entry table[], int package_size, cbor_buffer* buffer) {
    int no_entries_in_cbor = (package_size - BASE_CBOR_BYTE_SIZE) / CBOR_TARGET_STATE_MAX_BYTE_SIZE;
    if(no_entries_in_cbor <= 0) {
        return -1;
    }
    int no_cbor_streams = MAX_GATE_COUNT / no_entries_in_cbor;
    if(MAX_GATE_COUNT % no_entries_in_cbor != 0) {
        no_cbor_streams++;
    }

    int buffer_index = 0; //indicates where to write in buffer
    int table_index = 0; //inidcates current table entry

    while (table_index < MAX_GATE_COUNT)
    {
        for(int i = 0; i < no_cbor_streams; i++) { // i is index in table
            CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
            cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * (BASE_CBOR_BYTE_SIZE + CBOR_TARGET_STATE_MAX_BYTE_SIZE * no_cbor_streams), 0);
            cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
            cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
            cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, no_entries_in_cbor); // Entry 2

            // [Table Entry]
            if(MAX_GATE_COUNT - table_index < no_entries_in_cbor) {
                no_entries_in_cbor = MAX_GATE_COUNT - table_index;
            }
            for(int j = 0; j < no_entries_in_cbor; i++) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_simple_value(&singleEntryEncoder, table[table_index + j].gateID);
                cbor_encode_simple_value(&singleEntryEncoder, table[table_index + j].state);
                cbor_encode_int(&singleEntryEncoder, table[table_index + j].timestamp);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
                table_index++;
            }

            cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
            cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

            buffer->package_size[i] =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);
            buffer_index += buffer->package_size[i];
        }
    }
    return no_cbor_streams;
}