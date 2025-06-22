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

    #define BASE_CBOR_BYTE_SIZE 0x03
    #define CBOR_TARGET_STATE_MAX_BYTE_SIZE (0x0A + 0x01)
    #define CBOR_IS_STATE_MAX_BYTE_SIZE (0x0A + 0x01)
    #define CBOR_SEEN_STATUS_MAX_BYTE_SIZE (0x0C + 0x01)
    #define CBOR_JOBS_MAX_BYTE_SIZE (0x05 + 0x01)

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
        uint8_t* package_size;
        int capacity;
    } cbor_buffer;

    /**
     * @param buffer cbor buffer to write the cbor package into
     * @return 0 if successful
     * converts the table to a cbor package
    */
    int target_state_table_to_cbor(cbor_buffer* buffer);
    int is_state_table_to_cbor(cbor_buffer* buffer);
    int seen_status_table_to_cbor(cbor_buffer* buffer);
    int jobs_table_to_cbor(cbor_buffer* buffer);
    /**
     * @param package_size maximum size of one cbor package
     * @param buffer cbor buffer to write the cbor package into
     * @return number of cbor streams the table was converted to, -1 if an error occurred
     * turns the table into several cbor packages limited in size by the package_size parameter
    */
    int target_state_table_to_cbor_many(int package_size, cbor_buffer* buffer);
    int is_state_table_to_cbor_many(int package_size, cbor_buffer* buffer);
    int seen_status_table_to_cbor_many(int package_size, cbor_buffer* buffer);
    int jobs_table_to_cbor_many(int package_size, cbor_buffer* buffer);

    /**
     * @param buffer cbor buffer
     * @return 0 if successful, -1 otherwise
     * receives a cbor buffer and turns the sequence into table structs
     * consequently calls functions to merge received table with saved table
    */
    int cbor_to_table_test(cbor_buffer* buffer);

    /**
     * only for testing purposes
    */
    int target_state_table_to_cbor_test(target_state_entry table[], cbor_buffer* buffer);
    int target_state_table_to_cbor_many_test(target_state_entry table[], int package_size, cbor_buffer* buffer);
#endif