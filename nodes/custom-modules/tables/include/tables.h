/**
 * @brief       This module provides tables saved on the devices
 *
 * 
 * @author      
 */

#ifndef TABLES_H
#define TABLES_H
    #include <stdint.h>

    #define INVALID_GATE_ID (0xFF)
    #define MAX_GATE_COUNT (0xFF)

    #define TARGET_STATE_KEY 0x00
    #define IS_STATE_KEY 0x01
    #define SEEN_STATUS_KEY 0x02
    #define JOBS_KEY 0x03

    #define TABLE_SUCCESS               0
    #define TABLE_ERROR_SIZE_TOO_BIG   -1
    #define TABLE_ERROR_INVALID_GATE_ID -2
    #define TABLE_ERROR_NOT_FOUND      -3

    typedef struct {
        uint8_t gateID;
        uint8_t state;
        int timestamp;
    } target_state_entry;

    typedef struct {
        uint8_t gateID;
        uint8_t state;
        int gateTime;
    } is_state_entry;

    typedef struct {
        uint8_t gateID;
        int gateTime;
        uint8_t status;
        uint8_t senseMateID;
    } seen_status_entry;

    typedef struct {
        uint8_t gateID;
        uint8_t done;
    } jobs_entry;

    typedef struct {
        uint8_t* buffer;
        int cbor_size;
        int capacity;
    } cbor_buffer;

    /**
     * @param buffer cbor buffer to write the cbor package into
     * @return 0 if successful
     * converts the target state table to a cbor package
    */
    int target_state_table_to_cbor(cbor_buffer* buffer);

    int is_state_table_to_cbor(is_state_entry table[], cbor_buffer* buffer);

    int seen_status_table_to_cbor(seen_status_entry table[], cbor_buffer* buffer);

    int jobs_table_to_cbor(jobs_entry table[], cbor_buffer* buffer);

    int cbor_to_table(cbor_buffer* buffer);

    // TODO function to give back several CBOR packages for LoRaWAN

    int target_state_table_to_cbor_many(target_state_entry table[], int package_size, cbor_buffer* buffer);

    // Initialization
int init_tables(void);

// Setter functions (merge/update)
/**
 * Merge target state entries from another table
 * @param other Pointer to array of target_state_entry
 * @param size Number of entries in other array
 * @return TABLE_SUCCESS on success, error code on failure
 */
int merge_target_state_entry_table(const target_state_entry* other, uint8_t size);

/**
 * Merge is state entries from another table
 * @param other Pointer to array of is_state_entry
 * @param size Number of entries in other array
 * @return TABLE_SUCCESS on success, error code on failure
 */
int merge_is_state_entry_table(const is_state_entry* other, uint8_t size);

/**
 * Merge seen status entries from another table
 * @param other Pointer to array of seen_status_entry
 * @param size Number of entries in other array
 * @return TABLE_SUCCESS on success, error code on failure
 */
int merge_seen_status_entry_table(const seen_status_entry* other, uint8_t size);

/**
 * Merge jobs entries from another table
 * Note: Jobs are always overwritten (no timestamp comparison)
 * @param other Pointer to array of jobs_entry
 * @param size Number of entries in other array
 * @return TABLE_SUCCESS on success, error code on failure
 */
int merge_jobs_entry_table(const jobs_entry* other, uint8_t size);

/**
 * Set/update a single target state entry
 * Updates only if new entry has newer timestamp
 * @param entry Pointer to target_state_entry to set
 * @return TABLE_SUCCESS on success, error code on failure
 */
int set_target_state_entry(const target_state_entry* entry);

/**
 * Set/update a single is state entry
 * Updates only if new entry has newer gateTime
 * @param entry Pointer to is_state_entry to set
 * @return TABLE_SUCCESS on success, error code on failure
 */
int set_is_state_entry(const is_state_entry* entry);

/**
 * Set/update a single seen status entry
 * Updates only if new entry has newer gateTime
 * @param entry Pointer to seen_status_entry to set
 * @return TABLE_SUCCESS on success, error code on failure
 */
int set_seen_status_entry(const seen_status_entry* entry);

/**
 * Set/update a single jobs entry
 * Note: Jobs are always overwritten (no timestamp comparison)
 * @param entry Pointer to jobs_entry to set
 * @return TABLE_SUCCESS on success, error code on failure
 */
int set_jobs_entry(const jobs_entry* entry);

/**
 * Force set a target state entry (ignore timestamp)
 * Always overwrites existing entry regardless of timestamp
 * @param entry Pointer to target_state_entry to set
 * @return TABLE_SUCCESS on success, error code on failure
 */
int force_set_target_state_entry(const target_state_entry* entry);

// Getter functions
/**
 * Get a single target state entry by gate ID
 * @param gate_id Gate ID to look up
 * @param entry Pointer to target_state_entry to store result
 * @return TABLE_SUCCESS on success, TABLE_ERROR_NOT_FOUND if not found, error code on failure
 */
int get_target_state_entry(uint8_t gate_id, target_state_entry* entry);

/**
 * Get a single is state entry by gate ID
 * @param gate_id Gate ID to look up
 * @param entry Pointer to is_state_entry to store result
 * @return TABLE_SUCCESS on success, TABLE_ERROR_NOT_FOUND if not found, error code on failure
 */
int get_is_state_entry(uint8_t gate_id, is_state_entry* entry);

/**
 * Get a single seen status entry by gate ID
 * @param gate_id Gate ID to look up
 * @param entry Pointer to seen_status_entry to store result
 * @return TABLE_SUCCESS on success, TABLE_ERROR_NOT_FOUND if not found, error code on failure
 */
int get_seen_status_entry(uint8_t gate_id, seen_status_entry* entry);

/**
 * Get a single jobs entry by gate ID
 * @param gate_id Gate ID to look up
 * @param entry Pointer to jobs_entry to store result
 * @return TABLE_SUCCESS on success, TABLE_ERROR_NOT_FOUND if not found, error code on failure
 */
int get_jobs_entry(uint8_t gate_id, jobs_entry* entry);

/**
 * Get direct pointer to target state table 
 * @return Pointer to internal table array
 */
const target_state_entry* get_target_state_table(void);

/**
 * Get direct pointer to is state table 
 * @return Pointer to internal table array
 */
const is_state_entry* get_is_state_table(void);

/**
 * Get direct pointer to seen status table 
 * @return Pointer to internal table array
 */
const seen_status_entry* get_seen_status_table(void);

/**
 * Get direct pointer to jobs table 
 * @return Pointer to internal table array
 */
const jobs_entry* get_jobs_table(void);
#endif