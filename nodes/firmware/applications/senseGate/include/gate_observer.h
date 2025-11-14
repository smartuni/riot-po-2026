#pragma once

#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "event.h"
#include "event/timeout.h"
#include "periph/gpio.h"
#include "mate_types.h"

#ifndef GATE_OBSERVER_DISTANCE_SENSOR_CNT
#define GATE_OBSERVER_DISTANCE_SENSOR_CNT (1)
#endif

#ifndef GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT
#define GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT (1)
#endif

#ifndef GATE_OBSERVER_LIMITSWITCH_DEBOUNCE_MS
#define GATE_OBSERVER_LIMITSWITCH_DEBOUNCE_MS (2000)
#endif

typedef enum {
    LIMIT_SWITCH_ENGAGED,
    LIMIT_SWITCH_DISENGAGED,
    LIMIT_SWITCH_CONNECTION_ERROR,
} limit_switch_state_t;

typedef struct {
    uint32_t distances[GATE_OBSERVER_DISTANCE_SENSOR_CNT];
    limit_switch_state_t ls_states[GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT];
    gate_state_t concluded_state;
} gate_observer_state_t;

typedef struct {
    uint32_t closed_min;
    uint32_t closed_max;
    void *measure_cb_ctx;
    uint32_t (*measure_distance_cb)(void *ctx);
} distance_sensor_conf_t;

typedef struct {
    gpio_t pin;
    gpio_mode_t pull_conf;
    bool closed_level;
} limit_switch_pin_conf_t;

typedef struct {
    distance_sensor_conf_t distance_sensor_confs[GATE_OBSERVER_DISTANCE_SENSOR_CNT];
    limit_switch_pin_conf_t limit_switch_confs[GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT];
} gate_observer_config_t;

typedef void (*gate_state_changed_cb_t)(gate_state_t new_state);

typedef struct {
    gate_observer_config_t config;
    gate_observer_state_t state;
    event_t limit_switch_event;
    event_t ls_reactivate_evt;
    event_timeout_t ls_reactivate_evt_timeout;
    gate_state_changed_cb_t change_cb;
    bool ls_event_muted;
} gate_observer_t;

/**
 * @brief   Initialize the gate sensing module
 *
 * @param[in,out]  observer  observer instance to initialize (caller allocated)
 * @param[in]      config    configuration of the observer
 * @param[in]      cb        callback that is executed on state changes
 *
 * @return                   0 on success
 * @return                   negative value on error
 */
int gate_observer_init(gate_observer_t *observer, const gate_observer_config_t *config, gate_state_changed_cb_t cb);

/**
 * @brief   Get the currently concluded state
 *
 * @param[in]     observer   observer instance to operate on
 * @param[out]    state      where to store the currently observed state (can be NULL)
 *
 * @return                   The gate state concluded from all sensor readings
 */
gate_state_t gate_observer_get_state(gate_observer_t *observer, gate_observer_state_t *state);
