/**
 * @brief       This module defines common datatypes used for mate applications
 * 
 * @author      Michel Rottleuthner
 */
#ifndef MATE_TYPES_H
#define MATE_TYPES_H
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t gate_id_t;
typedef uint8_t mate_id_t;
typedef uint32_t mate_timestamp_t;

typedef enum {
    GATE_CLOSED = 0,
    GATE_OPEN = 1,
} gate_state_t;

typedef enum {
    JOB_IN_PROGRESS = 0,
    JOB_DONE = 1,
} job_state_t;

typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t gateID;
    gate_state_t state;
} gate_state_entry_t;

/* A gate state as requested by a pilot-worker.
 * The timestamp is always based on the gate sensor reported time. */
typedef gate_state_entry_t gate_target_state_entry_t;

/* A gate state as reported by a sensor.
 * The timestamp is always based on the gate sensor reported time. */
typedef gate_state_entry_t gate_sensor_state_entry_t;

/* A gate state as seen and reported by a worker.
 * The timestamp is always based on the
 * gate sensor reported time */
typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t gateID;
    gate_state_t state;
    mate_id_t senseMateID;
} gate_seen_state_entry_t;

typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t gateID;
    int8_t rssi;
} gate_timestamp_entry_t;

typedef struct {
    mate_timestamp_t timestamp;
    gate_id_t mateID;
    int8_t rssi;
} mate_seen_state_entry_t;

typedef struct {
    gate_id_t gateID;
    job_state_t done;
    uint8_t priority;
} gate_job_entry_t;

/* A struct which combines the local knowledge about a gate */
typedef struct {
    mate_timestamp_t sensor_timestamp;
    mate_timestamp_t target_timestamp;
    mate_timestamp_t beacon_timestamp;
    gate_state_t sensor_state;
    gate_state_t target_state;
    gate_id_t gateID;
    int8_t beacon_rssi;
    bool sensor_data_present : 1;
    bool target_data_present : 1;
    bool beacon_data_present : 1;
} gate_local_info_entry_t;

#endif
