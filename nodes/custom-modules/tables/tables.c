#include "tables.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "mutex.h"
#include "embUnit.h"
#include "embUnit/embUnit.h"
#include "fmt.h"
#include "cbor.h"
#include "container.h"
#include "mutex.h"

// Return Codes
#define TABLE_SUCCESS           0
#define TABLE_ERROR_SIZE_TOO_BIG -1
#define TABLE_ERROR_INVALID_GATE_ID -2
#define TABLE_ERROR_NOT_FOUND   -3

// Static tables
static target_state_entry target_state_entry_table[MAX_GATE_COUNT];
static is_state_entry is_state_entry_table[MAX_GATE_COUNT];
static seen_status_entry seen_status_entry_table[MAX_GATE_COUNT];
static jobs_entry jobs_entry_table[MAX_GATE_COUNT];

// Mutexes for thread safety
static mutex_t target_state_mutex = MUTEX_INIT;
static mutex_t is_state_mutex = MUTEX_INIT;
static mutex_t seen_status_mutex = MUTEX_INIT;
static mutex_t jobs_mutex = MUTEX_INIT;

int target_state_entry_to_cbor(target_state_entry entry, cbor_buffer buffer) {
    return 0;
}


/**
 * Initialize all tables with default values
 */
int init_tables() {
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
    if (size > MAX_GATE_COUNT) {
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
    if (size > MAX_GATE_COUNT) {
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
    if (size > MAX_GATE_COUNT) {
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
    if (size > MAX_GATE_COUNT) {
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

int get_all_target_state_entries(target_state_entry* entries, uint8_t* count) {
    if (entries == NULL || count == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&target_state_mutex);
    
    *count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_target_state_entry_present_internal(i)) {
            entries[*count] = target_state_entry_table[i];
            (*count)++;
        }
    }
    
    mutex_unlock(&target_state_mutex);
    return TABLE_SUCCESS;
}

int get_all_is_state_entries(is_state_entry* entries, uint8_t* count) {
    if (entries == NULL || count == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&is_state_mutex);
    
    *count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_is_state_entry_present_internal(i)) {
            entries[*count] = is_state_entry_table[i];
            (*count)++;
        }
    }
    
    mutex_unlock(&is_state_mutex);
    return TABLE_SUCCESS;
}

int get_all_seen_status_entries(seen_status_entry* entries, uint8_t* count) {
    if (entries == NULL || count == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&seen_status_mutex);
    
    *count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_seen_status_entry_present_internal(i)) {
            entries[*count] = seen_status_entry_table[i];
            (*count)++;
        }
    }
    
    mutex_unlock(&seen_status_mutex);
    return TABLE_SUCCESS;
}

int get_all_jobs_entries(jobs_entry* entries, uint8_t* count) {
    if (entries == NULL || count == NULL) {
        return TABLE_ERROR_INVALID_GATE_ID;
    }
    
    mutex_lock(&jobs_mutex);
    
    *count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_jobs_entry_present_internal(i)) {
            entries[*count] = jobs_entry_table[i];
            (*count)++;
        }
    }
    
    mutex_unlock(&jobs_mutex);
    return TABLE_SUCCESS;
}
