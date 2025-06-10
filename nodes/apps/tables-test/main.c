#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "tables.h"


void test_basic_operations(void) {
    printf("=== Basic Operations Test ===\n");
    
    // Initialize tables
    int result = init_tables();
    printf("init_tables(): %d (0=success)\n", result);
    
    // Test target_state entry
    target_state_entry target = {
        .gateID = 5,
        .state = 1,
        .timestamp = 1000
    };
    
    result = set_target_state_entry(&target);
    printf("set_target_state_entry(): %d\n", result);
    
    target_state_entry retrieved_target;
    result = get_target_state_entry(5, &retrieved_target);
    printf("get_target_state_entry(): %d\n", result);
    
    if (result == TABLE_SUCCESS) {
        printf("Retrieved: gateID=%d, state=%d, timestamp=%d\n", 
               retrieved_target.gateID, retrieved_target.state, retrieved_target.timestamp);
    }
    
    // Test not found
    target_state_entry not_found;
    result = get_target_state_entry(99, &not_found);
    printf("get_target_state_entry(99): %d (-3=not found)\n", result);
}

void test_timestamp_updates(void) {
    printf("\n=== Timestamp Update Test ===\n");
    
    // Add entry with timestamp 1000
    target_state_entry entry1 = {.gateID = 10, .state = 1, .timestamp = 1000};
    set_target_state_entry(&entry1);
    
    // Try to update with older timestamp (should be ignored)
    target_state_entry entry2 = {.gateID = 10, .state = 2, .timestamp = 500};
    set_target_state_entry(&entry2);
    
    target_state_entry retrieved;
    get_target_state_entry(10, &retrieved);
    printf("After older update: state=%d, timestamp=%d (should still be 1, 1000)\n", 
           retrieved.state, retrieved.timestamp);
    
    // Update with newer timestamp (should work)
    target_state_entry entry3 = {.gateID = 10, .state = 3, .timestamp = 2000};
    set_target_state_entry(&entry3);
    
    get_target_state_entry(10, &retrieved);
    printf("After newer update: state=%d, timestamp=%d (should be 3, 2000)\n", 
           retrieved.state, retrieved.timestamp);
}

void test_force_set(void) {
    printf("\n=== Force Set Test ===\n");
    
    // Add entry
    target_state_entry entry1 = {.gateID = 15, .state = 1, .timestamp = 2000};
    set_target_state_entry(&entry1);
    
    // Force set with older timestamp
    target_state_entry entry2 = {.gateID = 15, .state = 99, .timestamp = 100};
    force_set_target_state_entry(&entry2);
    
    target_state_entry retrieved;
    get_target_state_entry(15, &retrieved);
    printf("After force set: state=%d, timestamp=%d (should be 99, 100)\n", 
           retrieved.state, retrieved.timestamp);
}

void test_all_table_types(void) {
    printf("\n=== All Table Types Test ===\n");
    init_tables();
    // is_state
    is_state_entry is_entry = {.gateID = 20, .state = 1, .gateTime = 1000};
    int result = set_is_state_entry(&is_entry);
    printf("set_is_state_entry(): %d\n", result);
    
    is_state_entry retrieved_is;
    result = get_is_state_entry(20, &retrieved_is);
    printf("get_is_state_entry(): %d, gateID=%d, state=%d, gateTime=%d\n", 
           result, retrieved_is.gateID, retrieved_is.state, retrieved_is.gateTime);
    
    // seen_status
    seen_status_entry seen_entry = {.gateID = 25, .gateTime = 2000, .status = 255, .senseMateID = 42};
    result = set_seen_status_entry(&seen_entry);
    printf("set_seen_status_entry(): %d\n", result);
    
    seen_status_entry retrieved_seen;
    result = get_seen_status_entry(25, &retrieved_seen);
    printf("get_seen_status_entry(): %d, gateID=%d, gateTime=%d, status=%d, senseMateID=%d\n", 
           result, retrieved_seen.gateID, retrieved_seen.gateTime, 
           retrieved_seen.status, retrieved_seen.senseMateID);
    
    // jobs
    jobs_entry job_entry = {.gateID = 30, .done = 1};
    result = set_jobs_entry(&job_entry);
    printf("set_jobs_entry(): %d\n", result);
    
    jobs_entry retrieved_job;
    result = get_jobs_entry(30, &retrieved_job);
    printf("get_jobs_entry(): %d, gateID=%d, done=%d\n", 
           result, retrieved_job.gateID, retrieved_job.done);
}

void test_all_table_entries(void) {
    printf("\n=== ALL TABLE ENTRIES (POINTER VERSION) ===\n");
    uint8_t target_count = 0, is_count = 0, seen_count = 0, jobs_count = 0;
    
    // Erstmal alle Tabellen mit Testdaten füllen
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        target_state_entry target_entry = {
            .gateID = i,
            .state = i % 2,
            .timestamp = 1000 + i * 100
        };
        set_target_state_entry(&target_entry);
        
        is_state_entry is_state_entry = {
            .gateID = i,
            .state = i,
            .gateTime = 2000 + i * 50
        };
        set_is_state_entry(&is_state_entry);
        
        seen_status_entry seen_entry = {
            .gateID = i,
            .gateTime = 3000 + i,
            .status = i % 2,
            .senseMateID = 254 - i
        };
        set_seen_status_entry(&seen_entry);
        
        jobs_entry job_entry = {
            .gateID = i,
            .done = i % 2
        };
        set_jobs_entry(&job_entry);
    }
    
    // Direkte Pointer auf die internen Tabellen holen (LOCKFREE!)
    const target_state_entry* target_table = get_target_state_table();
    const is_state_entry* is_table = get_is_state_table();
    const seen_status_entry* seen_table = get_seen_status_table();
    const jobs_entry* jobs_table = get_jobs_table();
    
    // 1. Target State Tabelle
    printf("\n--- TARGET STATE TABLE ---\n");
    target_count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (target_table[i].gateID != MAX_GATE_COUNT) { // Entry ist valid
            target_count++;
        }
    }
    
    printf("Found %d target entries:\n", target_count);
    int entry_index = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (target_table[i].gateID != MAX_GATE_COUNT) {
            printf("  [%d] gateID=%d, state=%d, timestamp=%d\n",
                   entry_index, target_table[i].gateID, target_table[i].state, target_table[i].timestamp);
            entry_index++;
        }
    }
    
    // 2. Is State Tabelle
    printf("\n--- IS STATE TABLE ---\n");
    is_count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_table[i].gateID != MAX_GATE_COUNT) {
            is_count++;
        }
    }
    
    printf("Found %d is_state entries:\n", is_count);
    entry_index = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (is_table[i].gateID != MAX_GATE_COUNT) {
            printf("  [%d] gateID=%d, state=%d, gateTime=%d\n",
                   entry_index, is_table[i].gateID, is_table[i].state, is_table[i].gateTime);
            entry_index++;
        }
    }
    
    // 3. Seen Status Tabelle
    printf("\n--- SEEN STATUS TABLE ---\n");
    seen_count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (seen_table[i].gateID != MAX_GATE_COUNT) {
            seen_count++;
        }
    }
    
    printf("Found %d seen_status entries:\n", seen_count);
    entry_index = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (seen_table[i].gateID != MAX_GATE_COUNT) {
            printf("  [%d] gateID=%d, gateTime=%d, status=%d, senseMateID=%d\n",
                   entry_index, seen_table[i].gateID, seen_table[i].gateTime,
                   seen_table[i].status, seen_table[i].senseMateID);
            entry_index++;
        }
    }
    
    // 4. Jobs Tabelle
    printf("\n--- JOBS TABLE ---\n");
    jobs_count = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (jobs_table[i].gateID != MAX_GATE_COUNT) {
            jobs_count++;
        }
    }
    
    printf("Found %d jobs entries:\n", jobs_count);
    entry_index = 0;
    for (int i = 0; i < MAX_GATE_COUNT; i++) {
        if (jobs_table[i].gateID != MAX_GATE_COUNT) {
            printf("  [%d] gateID=%d, done=%d\n",
                   entry_index, jobs_table[i].gateID, jobs_table[i].done);
            entry_index++;
        }
    }
    
    // Zusammenfassung
    printf("\n--- SUMMARY ---\n");
    printf("Total entries: %d\n", target_count + is_count + seen_count + jobs_count);
    printf("  Target: %d\n", target_count);
    printf("  Is State: %d\n", is_count);
    printf("  Seen Status: %d\n", seen_count);
    printf("  Jobs: %d\n", jobs_count);
}

void test_error_cases(void) {
    printf("\n=== Error Cases Test ===\n");
    
    // Invalid gate ID
    target_state_entry invalid_entry = {.gateID = 255, .state = 1, .timestamp = 1000};
    int result = set_target_state_entry(&invalid_entry);
    printf("set_target_state_entry(gateID=255): %d (-2=invalid gate ID)\n", result);
    
    // NULL pointer
    result = set_target_state_entry(NULL);
    printf("set_target_state_entry(NULL): %d (-2=invalid gate ID)\n", result);
    
    // Get with NULL pointer
    result = get_target_state_entry(5, NULL);
    printf("get_target_state_entry(5, NULL): %d (-2=invalid gate ID)\n", result);
}

int main(void) {
    ztimer_sleep(ZTIMER_MSEC, 5000);
    puts("Starting table tests...\n\n");
    // test_basic_operations();
    // test_timestamp_updates();
    // test_force_set();
    // test_all_table_types();
    test_all_table_entries();
    //test_error_cases();
    
    printf("\n=== Tests Complete ===\n");
    while (1) {
        ztimer_sleep(ZTIMER_MSEC, 1000);
    }
    return 0;
}