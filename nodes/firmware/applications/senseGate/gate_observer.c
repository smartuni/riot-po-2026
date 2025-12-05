#include <string.h>
#include "gate_observer.h"
#include "event/thread.h"

event_queue_t *_observer_event_queue = NULL;
static char observer_stack[2*THREAD_STACKSIZE_DEFAULT];

static void _update_state(gate_observer_t *observer)
{
    int close_cnt = 0;

    for (int i = 0; i < GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT; i++) {
        limit_switch_pin_conf_t *lsc = &observer->config.limit_switch_confs[i];
        bool closed = gpio_read(lsc->pin) == lsc->closed_level;
        observer->state.ls_states[i] = closed ? LIMIT_SWITCH_ENGAGED : LIMIT_SWITCH_DISENGAGED;
        if (closed) {
            close_cnt++;
        }
    }

    for (int i = 0; i < GATE_OBSERVER_DISTANCE_SENSOR_CNT; i++) {
        distance_sensor_conf_t *dsc = &observer->config.distance_sensor_confs[i];
        uint32_t distance = dsc->measure_distance_cb(dsc->measure_cb_ctx);
        observer->state.distances[i] = distance;
        bool closed = (dsc->closed_min <= distance) && (dsc->closed_max >= distance);
        if (closed) {
            close_cnt++;
        }
    }

    gate_state_t prev_state = observer->state.concluded_state;
    /* for now only consider the gate closed if all sensors indicate a closed state */
    if (close_cnt == (GATE_OBSERVER_DISTANCE_SENSOR_CNT + GATE_OBSERVER_DISTANCE_SENSOR_CNT)) {
        observer->state.concluded_state = GATE_STATE_CLOSED;
    } else {
        observer->state.concluded_state = GATE_STATE_OPEN;
    }

    if (prev_state != observer->state.concluded_state) {
        observer->change_cb(observer->state.concluded_state);
    }
}

void limit_switch_event_handler(event_t *event)
{
    gate_observer_t *observer = container_of(event, gate_observer_t, limit_switch_event);
    if (!observer->ls_event_muted) {
        _update_state(observer);
        observer->ls_event_muted = true;
        /* reactivate the event after the debounce */
        event_timeout_set(&observer->ls_reactivate_evt_timeout,
                          GATE_OBSERVER_LIMITSWITCH_DEBOUNCE_MS);
    }
}

void limit_switch_reactivate_handler(event_t *event)
{
    gate_observer_t *observer = container_of(event, gate_observer_t, ls_reactivate_evt);
    observer->ls_event_muted = false;
}

void limit_switch_cb(void *arg) {
    event_t *evt = (event_t*)arg;
    if (_observer_event_queue) {
        event_post(_observer_event_queue, evt);
    }
}

void* observer_thread(void* arg)
{
    gate_observer_t *observer = (gate_observer_t*)arg;

    event_queue_t observer_event_queue;
    _observer_event_queue = &observer_event_queue;
    event_queue_init(&observer_event_queue);

    event_timeout_ztimer_init(&observer->ls_reactivate_evt_timeout,
                              ZTIMER_MSEC, &observer_event_queue,
                              &observer->ls_reactivate_evt);

    event_loop(&observer_event_queue);

    return NULL;
}

int gate_observer_init(gate_observer_t *observer, const gate_observer_config_t *config, gate_state_changed_cb_t cb)
{
    assert(observer && config);

    memcpy(&observer->config, config, sizeof(gate_observer_config_t));
    observer->change_cb = cb;
    //TODO: replace with explicit "invalid" value
    observer->state.concluded_state = 0xFF;

    observer->limit_switch_event.handler = limit_switch_event_handler;
    observer->ls_reactivate_evt.handler = limit_switch_reactivate_handler;
    observer->ls_event_muted = false;

    thread_create(
        observer_stack,
        sizeof(observer_stack),
        THREAD_PRIORITY_MAIN - 3,
        THREAD_CREATE_STACKTEST,
        observer_thread,
        observer,
       "gate_observer"
    );

    for (int i = 0; i < GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT; i++) {
        limit_switch_pin_conf_t *lsc = &observer->config.limit_switch_confs[i];
        if (gpio_init_int(lsc->pin, lsc->pull_conf, GPIO_BOTH, limit_switch_cb,
                          &observer->limit_switch_event) != 0) {
            return -1;
        }
    }
    _update_state(observer);
    return 0;
}

gate_state_t gate_observer_get_state(gate_observer_t *observer, gate_observer_state_t *state)
{
    _update_state(observer);
    if (state) {
        memcpy(state, &observer->state, sizeof(gate_observer_state_t));
    }
    return observer->state.concluded_state;
}
