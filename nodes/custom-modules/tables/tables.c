#include "include/tables.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mutex.h"
#include <stddef.h>
#include "embUnit.h"
#include "embUnit/embUnit.h"
#include "fmt.h"
#include "cbor.h"
#include "container.h"

// Return Codes
#define TABLE_SUCCESS           0
#define TABLE_ERROR_SIZE_TOO_BIG -1
#define TABLE_ERROR_INVALID_GATE_ID -2
#define TABLE_ERROR_NOT_FOUND   -3

// Mutexes for thread safety
static mutex_t target_state_mutex = MUTEX_INIT;
static mutex_t is_state_mutex = MUTEX_INIT;
static mutex_t seen_status_mutex = MUTEX_INIT;
static mutex_t jobs_mutex = MUTEX_INIT;

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

/**
 * Initialize all tables with default values
 */
int init_tables(void) {
    mutex_lock(&target_state_mutex);
    mutex_lock(&is_state_mutex);
    mutex_lock(&seen_status_mutex);
    mutex_lock(&jobs_mutex);
    
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        target_state_entry_table[i].gateID = MAX_GATE_COUNT;  // Mark as empty
        is_state_entry_table[i].gateID = MAX_GATE_COUNT;
        seen_status_entry_table[i].gateID = MAX_GATE_COUNT;
        jobs_entry_table[i].gateID = MAX_GATE_COUNT;
    }
    
    mutex_unlock(&jobs_mutex);
    mutex_unlock(&seen_status_mutex);
    mutex_unlock(&is_state_mutex);
    mutex_unlock(&target_state_mutex);
    
    return TABLE_SUCCESS;
}

static int is_target_state_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;  // Invalid gate_id = not present
    }
    return target_state_entry_table[gate_id].gateID != MAX_GATE_COUNT;
}

static int is_is_state_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    return is_state_entry_table[gate_id].gateID != MAX_GATE_COUNT;
}

static int is_seen_status_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    return seen_status_entry_table[gate_id].gateID != MAX_GATE_COUNT;
}

static int is_jobs_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    return jobs_entry_table[gate_id].gateID != MAX_GATE_COUNT;
}

static inline int is_valid_gate_id(uint8_t gate_id) {
    return gate_id < MAX_GATE_COUNT;
}

int set_target_state_entry(const target_state_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&target_state_mutex);
    
    if (!is_target_state_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        target_state_entry_table[gate_id] = *entry;
    }
    else if (target_state_entry_table[gate_id].timestamp < entry->timestamp) {
        // New entry is newer, update ours
        target_state_entry_table[gate_id] = *entry;
    }
    
    mutex_unlock(&target_state_mutex);
    return TABLE_SUCCESS;
}

int set_is_state_entry(const is_state_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&is_state_mutex);
    
    if (!is_is_state_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        is_state_entry_table[gate_id] = *entry;
    }
    else if (is_state_entry_table[gate_id].gateTime < entry->gateTime) {
        // New entry is newer, update ours
        is_state_entry_table[gate_id] = *entry;
    }
    
    mutex_unlock(&is_state_mutex);
    return TABLE_SUCCESS;
}

int set_seen_status_entry(const seen_status_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&seen_status_mutex);
    
    if (!is_seen_status_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        seen_status_entry_table[gate_id] = *entry;
    }
    else if (seen_status_entry_table[gate_id].gateTime < entry->gateTime) {
        // New entry is newer, update ours
        seen_status_entry_table[gate_id] = *entry;
    }
    
    mutex_unlock(&seen_status_mutex);
    return TABLE_SUCCESS;
}

int set_jobs_entry(const jobs_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&jobs_mutex);
    jobs_entry_table[gate_id] = *entry;
    mutex_unlock(&jobs_mutex);
    
    return TABLE_SUCCESS;
}

int force_set_target_state_entry(const target_state_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&target_state_mutex);
    target_state_entry_table[gate_id] = *entry;
    mutex_unlock(&target_state_mutex);
    
    return TABLE_SUCCESS;
}

int merge_target_state_entry_table(const target_state_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    
    for (int i = 0; i < size; i++) {
        int result = set_target_state_entry(&other[i]);
        if (result != TABLE_SUCCESS && result != TABLE_ERROR_INVALID_GATE_ID) {
            return result; // Propagate unexpected errors
        }
    }
    return TABLE_SUCCESS;
}

int merge_is_state_entry_table(const is_state_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    
    for (int i = 0; i < size; i++) {
        int result = set_is_state_entry(&other[i]);
        if (result != TABLE_SUCCESS && result != TABLE_ERROR_INVALID_GATE_ID) {
            return result;
        }
    }
    return TABLE_SUCCESS;
}

int merge_seen_status_entry_table(const seen_status_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    
    for (int i = 0; i < size; i++) {
        int result = set_seen_status_entry(&other[i]);
        if (result != TABLE_SUCCESS && result != TABLE_ERROR_INVALID_GATE_ID) {
            return result;
        }
    }
    return TABLE_SUCCESS;
}

int merge_jobs_entry_table(const jobs_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    
    for (int i = 0; i < size; i++) {
        int result = set_jobs_entry(&other[i]);
        if (result != TABLE_SUCCESS && result != TABLE_ERROR_INVALID_GATE_ID) {
            return result;
        }
    }
    return TABLE_SUCCESS;
}

int get_target_state_entry(uint8_t gate_id, target_state_entry* entry) {
    if (entry == NULL || !is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&target_state_mutex);
    
    if (!is_target_state_entry_present_internal(gate_id)) {
        mutex_unlock(&target_state_mutex);
        return TABLE_ERROR_NOT_FOUND;
    }
    
    *entry = target_state_entry_table[gate_id];
    mutex_unlock(&target_state_mutex);
    
    return TABLE_SUCCESS;
}

int get_is_state_entry(uint8_t gate_id, is_state_entry* entry) {
    if (entry == NULL || !is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&is_state_mutex);
    
    if (!is_is_state_entry_present_internal(gate_id)) {
        mutex_unlock(&is_state_mutex);
        return TABLE_ERROR_NOT_FOUND;
    }
    
    *entry = is_state_entry_table[gate_id];
    mutex_unlock(&is_state_mutex);
    
    return TABLE_SUCCESS;
}

int get_seen_status_entry(uint8_t gate_id, seen_status_entry* entry) {
    if (entry == NULL || !is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&seen_status_mutex);
    
    if (!is_seen_status_entry_present_internal(gate_id)) {
        mutex_unlock(&seen_status_mutex);
        return TABLE_ERROR_NOT_FOUND;
    }
    
    *entry = seen_status_entry_table[gate_id];
    mutex_unlock(&seen_status_mutex);
    
    return TABLE_SUCCESS;
}

int get_jobs_entry(uint8_t gate_id, jobs_entry* entry) {
    if (entry == NULL || !is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&jobs_mutex);
    
    if (!is_jobs_entry_present_internal(gate_id)) {
        mutex_unlock(&jobs_mutex);
        return TABLE_ERROR_NOT_FOUND;
    }
    
    *entry = jobs_entry_table[gate_id];
    mutex_unlock(&jobs_mutex);
    
    return TABLE_SUCCESS;
}

const target_state_entry* get_target_state_table(void) {
    return target_state_entry_table;
}

const is_state_entry* get_is_state_table(void) {
    return is_state_entry_table;
}

const seen_status_entry* get_seen_status_table(void) {
    return seen_status_entry_table;
}

const jobs_entry* get_jobs_table(void) {
    return jobs_entry_table;
}