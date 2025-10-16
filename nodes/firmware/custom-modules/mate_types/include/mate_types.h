/**
 * @brief       This module defines common datatypes used for mate applications
 * 
 * @author      Michel Rottleuthner
 */
#ifndef MATE_TYPES_H
#define MATE_TYPES_H

typedef enum {
    GATE_CLOSED = 0,
    GATE_OPEN = 1,
} gate_state_t;

typedef struct {
    uint8_t gateID;
    uint8_t state;
    int timestamp;
} target_state_entry;

typedef struct {
    uint8_t gateID;
    uint8_t state;
    uint32_t gateTime;
} is_state_entry;

typedef struct {
    uint8_t gateID;
    int gateTime;
    uint8_t status;
    uint8_t senseMateID;
} seen_status_entry;

typedef struct {
    uint8_t gateID;
    uint32_t timestamp;
    int8_t rssi;
} timestamp_entry;

typedef struct {
    uint8_t gateID;
    uint8_t done;
    uint8_t priority;
} jobs_entry;

#endif
