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
#endif