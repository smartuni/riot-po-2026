/**
 * @brief       This module defines common datatypes used for mate applications
 * 
 * @author      Michel Rottleuthner
 */
#ifndef MATE_TYPES_H
#define MATE_TYPES_H

typedef uint8_t gate_id_t;
typedef uint8_t mate_id_t;
typedef uint32_t mate_timestamp_t;

typedef enum {
    GATE_CLOSED = 0,
    GATE_OPEN = 1,
} gate_state_t;

typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t gateID;
    gate_state_t state;
} target_state_entry;

typedef struct {
    mate_timestamp_t gateTime;
    gate_id_t gateID;
    gate_state_t state;
} is_state_entry;

typedef struct {
    mate_timestamp_t gateTime;
    gate_id_t gateID;
    gate_state_t status;
    mate_id_t senseMateID;
} seen_status_entry;

typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t gateID;
    int8_t rssi;
} timestamp_entry;

typedef struct {
    gate_id_t gateID;
    uint8_t done;
    uint8_t priority;
} jobs_entry;

#endif
