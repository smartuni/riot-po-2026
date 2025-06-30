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

// Static tables
static target_state_entry target_state_entry_table[MAX_GATE_COUNT];
static int target_state_entry_count = 0;
static is_state_entry is_state_entry_table[MAX_GATE_COUNT];
static int is_state_entry_count = 0;
static seen_status_entry seen_status_entry_table[MAX_GATE_COUNT];
static int seen_status_entry_count = 0;
static jobs_entry jobs_entry_table[MAX_GATE_COUNT];
static timestamp_entry timestamp_table[MAX_GATE_COUNT];
static int jobs_entry_count = 0;

// Mutexes for thread safety
static mutex_t target_state_mutex = MUTEX_INIT;
static mutex_t is_state_mutex = MUTEX_INIT;
static mutex_t seen_status_mutex = MUTEX_INIT;
static mutex_t jobs_mutex = MUTEX_INIT;
static mutex_t timestamp_mutex = MUTEX_INIT;

/**
 * Initialize all tables with default values
 */
int init_tables(void) {
    mutex_lock(&target_state_mutex);
    mutex_lock(&is_state_mutex);
    mutex_lock(&seen_status_mutex);
    mutex_lock(&jobs_mutex);
    mutex_lock(&timestamp_mutex);
    
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        target_state_entry_table[i].gateID = MAX_GATE_COUNT;  // Mark as empty
        is_state_entry_table[i].gateID = MAX_GATE_COUNT;
        seen_status_entry_table[i].gateID = MAX_GATE_COUNT;
        jobs_entry_table[i].gateID = MAX_GATE_COUNT;
        timestamp_table[i].gateID = MAX_GATE_COUNT;
    }
    
    mutex_unlock(&timestamp_mutex);
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
    uint8_t entry_gate_id = target_state_entry_table[gate_id].gateID;
    return entry_gate_id != MAX_GATE_COUNT && entry_gate_id == gate_id;
}

static int is_is_state_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    uint8_t entry_gate_id = is_state_entry_table[gate_id].gateID;
    return entry_gate_id != MAX_GATE_COUNT && entry_gate_id == gate_id;
}

static int is_seen_status_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    uint8_t entry_gate_id = seen_status_entry_table[gate_id].gateID;
    return entry_gate_id != MAX_GATE_COUNT && entry_gate_id == gate_id;
}

static int is_jobs_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    uint8_t entry_gate_id = jobs_entry_table[gate_id].gateID;
    return entry_gate_id != MAX_GATE_COUNT && entry_gate_id == gate_id;
}

static int is_timestamp_entry_present_internal(uint8_t gate_id) {
    if (gate_id >= MAX_GATE_COUNT) {
        return 0;
    }
    uint8_t entry_gate_id = timestamp_table[gate_id].gateID;
    return entry_gate_id != MAX_GATE_COUNT && entry_gate_id == gate_id;
}



static inline int is_valid_gate_id(uint8_t gate_id) {
    return gate_id < MAX_GATE_COUNT;
}

int target_state_table_to_cbor_test(target_state_entry table[], cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 3); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encode_int(&arrayEncoder, 2008); // Entry 2
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 4); // Entry 3

    // [Table Entry]
    for(int i = 0; i < 4; i++) {
        if (table[i].gateID != MAX_GATE_COUNT) {
            cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
            cbor_encode_int(&singleEntryEncoder, table[i].gateID);
            cbor_encode_int(&singleEntryEncoder, table[i].state);
            cbor_encode_int(&singleEntryEncoder, table[i].timestamp);
            cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
        }
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size = (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int target_state_table_to_cbor(cbor_buffer* buffer) {
    if (target_state_entry_count == 0){
        return -1; // No entries in table
    }
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, target_state_entry_count); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_target_state_entry_present_internal(i)) {
            cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
            cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].gateID);
            cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].state);
            cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].timestamp);
            cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
        }
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int is_state_table_to_cbor(cbor_buffer* buffer) {
    if (is_state_entry_count == 0){
        return -1; // No entries in table
    }
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, is_state_entry_count); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        if(is_is_state_entry_present_internal(i)) {
            cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
            cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].gateID);
            cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].state);
            cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].gateTime);
            cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
        }
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int seen_status_table_to_cbor(cbor_buffer* buffer) {
    if (seen_status_entry_count == 0){
        return -1; // No entries in table
    }
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, seen_status_entry_count); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        if(is_seen_status_entry_present_internal(i)) {
            cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
            cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].gateID);
            cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].gateTime);
            cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].status);
            cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].senseMateID);
            cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
        }
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int jobs_table_to_cbor(cbor_buffer* buffer) {
    if (jobs_entry_count == 0){
        return -1; // No jobs in table
    }
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, jobs_entry_count); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_jobs_entry_present_internal(i)) {
            cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
            cbor_encode_int(&singleEntryEncoder, jobs_entry_table[i].gateID);
            cbor_encode_int(&singleEntryEncoder, jobs_entry_table[i].done);
            cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
        }
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int cbor_to_table_test(cbor_buffer* buffer) {
    CborParser parser;
    CborValue value;
    CborValue wrapperValue;
    CborValue fieldsValue;
    CborValue entryValue;

    int tableType, timeStamp;
    target_state_entry returnTargetTable[buffer->cbor_size];
    is_state_entry returnIsTable[buffer->cbor_size];
    seen_status_entry returnSeenTable[buffer->cbor_size];
    jobs_entry returnJobsTable[buffer->cbor_size];

    cbor_parser_init(buffer->buffer, buffer->cbor_size, 0, &parser, &value);
    
    if(cbor_value_enter_container(&value, &wrapperValue) != CborNoError) {
        return -1;
    }

    if(!cbor_value_is_integer(&wrapperValue) || cbor_value_get_int(&wrapperValue, &tableType) != CborNoError) {
        return -1;
    } // get type of table
    if(tableType == TARGET_STATE_KEY) {
        cbor_value_advance(&wrapperValue);
        if(!cbor_value_is_integer(&wrapperValue) || cbor_value_get_int(&wrapperValue, &timeStamp) != CborNoError) {
            return -1;
        } // get timestamp
    }

    // [ enter second container
    cbor_value_advance(&wrapperValue);
    if(cbor_value_enter_container(&wrapperValue, &fieldsValue) != CborNoError) {
        return -1;
    }

    int id, s, sID, p, gt;
    size_t length = 0;
    cbor_value_get_array_length(&wrapperValue, &length); 	
    for(size_t i = 0; i < length; i++) {
        cbor_value_enter_container(&fieldsValue, &entryValue); // [
        switch(tableType) {
            case TARGET_STATE_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                target_state_entry newTargetEntry = {id, s, timeStamp};
                returnTargetTable[i] = newTargetEntry;
                break;
            case IS_STATE_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &gt) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                is_state_entry newIsEntry = {id, s, gt};
                returnIsTable[i] = newIsEntry;
                break;
            case SEEN_STATUS_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &gt) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &sID) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                seen_status_entry newSeenEntry = {id, gt, s, sID};
                returnSeenTable[i] = newSeenEntry;
                break;
            case JOBS_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &p) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                jobs_entry newJobsEntry = {id, JOB_IN_PROGRESS, p};
                returnJobsTable[i] = newJobsEntry;
                break;
        }
        cbor_value_leave_container(&fieldsValue,&entryValue); // ]
    }

    cbor_value_leave_container(&wrapperValue, &fieldsValue); // ]	
    cbor_value_leave_container(&value, &wrapperValue); // ]	

    // Integrate local data into global table
    switch(tableType) {
            case TARGET_STATE_KEY:
                merge_target_state_entry_table(returnTargetTable, (length-1));
                break;
            case IS_STATE_KEY:
                merge_is_state_entry_table(returnIsTable, (length-1));
                break;
            case SEEN_STATUS_KEY:
                merge_seen_status_entry_table(returnSeenTable, (length-1));
                break;
            case JOBS_KEY:
                merge_jobs_entry_table(returnJobsTable, (length-1));
                break;
            default:
                return -1;
    }

    return 0;
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
    int res = TABLE_NO_UPDATES;
    if (!is_target_state_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        target_state_entry_count++;
        target_state_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    else if (target_state_entry_table[gate_id].timestamp < entry->timestamp) {
        // New entry is newer, update ours
        target_state_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    
    mutex_unlock(&target_state_mutex);
    return res;
}

int set_is_state_entry(const is_state_entry* entry) {
    printf("Called: set_is_state_entry\n");
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&is_state_mutex);
    int res = TABLE_NO_UPDATES;
    if (!is_is_state_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        is_state_entry_count++;
        is_state_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    else if (is_state_entry_table[gate_id].gateTime < entry->gateTime) {
        // New entry is newer, update ours
        is_state_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    
    mutex_unlock(&is_state_mutex);
    return res;
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
    int res = TABLE_NO_UPDATES;
    if (!is_seen_status_entry_present_internal(gate_id)) {
        // Entry doesn't exist yet, add it
        seen_status_entry_count++;
        seen_status_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    else if (seen_status_entry_table[gate_id].gateTime < entry->gateTime) {
        // New entry is newer, update ours
        seen_status_entry_table[gate_id] = *entry;
        res = TABLE_UPDATED;
    }
    
    mutex_unlock(&seen_status_mutex);
    return res;
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
    if (jobs_entry_table[gate_id].gateID != MAX_GATE_COUNT) {
        jobs_entry_count++;
    }
    jobs_entry_table[gate_id] = *entry;
    mutex_unlock(&jobs_mutex);
    
    return TABLE_UPDATED;
}

int set_timestamp_entry(const timestamp_entry* entry) {
    if (entry == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    uint8_t gate_id = entry->gateID;
    if (!is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&timestamp_mutex);
    timestamp_table[gate_id] = *entry;
    mutex_unlock(&timestamp_mutex);
    
    return TABLE_UPDATED;
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
    
    return TABLE_UPDATED;;
}

int merge_target_state_entry_table(const target_state_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    int merge_result = TABLE_NO_UPDATES;
    for (int i = 0; i < size; i++) {
        int result = set_target_state_entry(&other[i]);
        if (TABLE_UPDATED == result) {
            merge_result = result; // Propagate unexpected errors
        }
    }
    return merge_result;
}

int merge_is_state_entry_table(const is_state_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    int merge_result = TABLE_NO_UPDATES;
    for (int i = 0; i < size; i++) {
        int result = set_is_state_entry(&other[i]);
        if (TABLE_UPDATED == result) {
            merge_result = result; // Propagate unexpected errors
        }
    }
    return merge_result;
}

int merge_seen_status_entry_table(const seen_status_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    int merge_result = TABLE_NO_UPDATES;
    for (int i = 0; i < size; i++) {
        int result = set_seen_status_entry(&other[i]);
        if (TABLE_UPDATED == result) {
            merge_result = result; // Propagate unexpected errors
        }
    }
    return merge_result;
}

int merge_jobs_entry_table(const jobs_entry* other, uint8_t size) {
    if (size >= MAX_GATE_COUNT) {
        return TABLE_ERROR_SIZE_TOO_BIG;
    }
    int merge_result = TABLE_NO_UPDATES;
    for (int i = 0; i < size; i++) {
        int result = set_jobs_entry(&other[i]);
        if (TABLE_UPDATED == result) {
            merge_result = result; // Propagate unexpected errors
        }
    }
    return merge_result;
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

int get_timestamp_entry(uint8_t gate_id, timestamp_entry* entry) {
    if (entry == NULL || !is_valid_gate_id(gate_id)) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&timestamp_mutex);
    
    if (!is_timestamp_entry_present_internal(gate_id)) {
        mutex_unlock(&timestamp_mutex);
        return TABLE_ERROR_NOT_FOUND;
    }
    
    *entry = timestamp_table[gate_id];
    mutex_unlock(&timestamp_mutex);
    
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
 
const timestamp_entry* get_timestamp_table(void) {
    return timestamp_table;
}

int target_state_table_to_cbor_many_test(target_state_entry table[], int package_size, cbor_buffer* buffer) {
    printf("Entered function\n");
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_TARGET_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        printf("%d + %d < %d\n", BASE_CBOR_BYTE_SIZE, CBOR_TARGET_STATE_MAX_BYTE_SIZE, package_size);
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    printf("This is the buffer: %p\n", buffer->buffer);
    while(table_index < 4) {
        printf("enterd while, i = %d, size = %d\n", table_index, size_of_current_cbor);
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        printf("Space = %p\n", space);
        printf("CBOR Index = %d\n", cbor_stream_index);
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 6); // Entry 2
        printf("while %d + %d < %d\n", size_of_current_cbor, CBOR_TARGET_STATE_MAX_BYTE_SIZE, package_size);
        while(size_of_current_cbor + CBOR_TARGET_STATE_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(table[table_index].gateID != MAX_GATE_COUNT) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, table[table_index].timestamp);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
            printf("Entered one entry, size = %d\n", size_of_current_cbor);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int target_state_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_TARGET_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while((target_state_entry_count > 0) && (table_index < MAX_GATE_COUNT)) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, target_state_entry_count); // Entry 2
        while((size_of_current_cbor + CBOR_TARGET_STATE_MAX_BYTE_SIZE < package_size) && (table_index < MAX_GATE_COUNT)) {
            if (target_state_entry_table[table_index].gateID != MAX_GATE_COUNT) {
                printf("Valid entry: %d\n", target_state_entry_table[table_index].gateID);
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].timestamp);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int is_state_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    printf("Called: is_state_table_to_cbor_many\n");
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_IS_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    int is_states_entry_processed = 0;
    int calculated_array_entries = (package_size - BASE_CBOR_BYTE_SIZE) / CBOR_IS_STATE_MAX_BYTE_SIZE;
    printf("calculated_array_entries: %d\n", calculated_array_entries);
    while((is_state_entry_count > 0) && (table_index < MAX_GATE_COUNT) && (is_state_entry_count - is_states_entry_processed) > 0) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        while( ((is_states_entry_processed % calculated_array_entries) < calculated_array_entries)&& (table_index < MAX_GATE_COUNT) && (is_state_entry_count - is_states_entry_processed) > 0) {
            printf("is_states_entry_processed modulo calculated_array_entries: %d\n", (is_states_entry_processed % calculated_array_entries));
            printf("package size: %d\n", package_size);
            printf("size_of_current_cbor + CBOR_IS_STATE_MAX_BYTE_SIZE + BASE_CBOR_BYTE_SIZE: %d\n", size_of_current_cbor + CBOR_IS_STATE_MAX_BYTE_SIZE + BASE_CBOR_BYTE_SIZE);
            if(size_of_current_cbor == 0 ){
                printf("Creating new cbor\n");
                cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
                cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
                cbor_encode_int(&arrayEncoder, IS_STATE_KEY);
                if(is_state_entry_count - is_states_entry_processed < calculated_array_entries){
                    printf("Rest of entries smaller than max sendable entries\n");
                    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, is_state_entry_count - is_states_entry_processed);  
                }else{
                    printf("Rest of entries bigger than max sendable entries\n");
                    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder,calculated_array_entries); 
                }
            } // Entry 1
               
            if (is_state_entry_table[table_index].gateID != MAX_GATE_COUNT) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].gateTime);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
                is_states_entry_processed++;
            }
            printf("table index: %d\n",table_index);
            table_index++;
            
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
            printf("size_of_current_cbor: %d\n",size_of_current_cbor);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int seen_status_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_SEEN_STATUS_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while((seen_status_entry_count > 0) && (table_index < MAX_GATE_COUNT)) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, SEEN_STATUS_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, seen_status_entry_count); // Entry 2
        while((size_of_current_cbor + CBOR_SEEN_STATUS_MAX_BYTE_SIZE < package_size) && (table_index < MAX_GATE_COUNT)) {
            if(seen_status_entry_table[table_index].gateID != MAX_GATE_COUNT) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 4); // []
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].gateTime);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].senseMateID);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].status);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int jobs_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_JOBS_MAX_BYTE_SIZE> package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while((jobs_entry_count > 0) && (table_index < MAX_GATE_COUNT)) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, JOBS_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, jobs_entry_count); // Entry 2
        while((size_of_current_cbor + CBOR_JOBS_MAX_BYTE_SIZE < package_size) && (table_index < MAX_GATE_COUNT)) {
            //validate table entry
            if(jobs_entry_table[table_index].gateID != MAX_GATE_COUNT) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 2); // []
                cbor_encode_int(&singleEntryEncoder, jobs_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, jobs_entry_table[table_index].done);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}