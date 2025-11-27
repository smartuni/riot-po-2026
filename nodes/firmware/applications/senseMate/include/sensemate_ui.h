#ifndef SENSEMATE_UI_H
#define SENSEMATE_UI_H

#include "tables/types.h"

typedef enum {
    DISCONNECTED,
    ESTABLISHING_CONNECTION,
    CONNECTED,
    RECEIVED,
    TRANSMITTED,
} ui_connection_state_t;

/* A struct which combines the local knowledge about a gate */
typedef struct {
    gate_state_t sensor_state;
    gate_state_t target_state;
    node_id_t gateID;
    int8_t beacon_rssi;
    bool sensor_data_present : 1;
    bool target_data_present : 1;
    bool beacon_data_present : 1;
} gate_local_info_entry_t;

/* type used to pass a gate state to the UI */
typedef struct {
    union {
        //TODO: replace local_gate_info type with
        //      something more appropriate for new tables API
        gate_local_info_entry_t local_gate_info;
        table_gate_observation_t gate_observation;
    } data;
    union {
        void *ptr;
        int  idx;
    } iter_ctx;
} ui_data_element_t;

/*
 * Generic callback type used for iterating over objects to be displayed.
 * @param  prev   caller allocated ui_data_element_t
 *                set iter_ctx.ptr == NULL to start iteration.
 * @return true  if there was a next element
 *         false if iteration finished
 */
typedef bool (*ui_data_element_iter_cb_t)(ui_data_element_t *prev);

typedef bool (*ui_create_data_element_cb_t)(ui_data_element_t *element);

typedef struct {
    ui_data_element_iter_cb_t all_gates_iter;
    ui_data_element_iter_cb_t jobs_iter;
    ui_create_data_element_cb_t put_gate_observation;
} ui_data_cbs_t;

typedef struct {
    uint16_t visible_gate_cnt;
    uint16_t pending_jobs_cnt;
    uint16_t visible_mate_cnt;
    ui_connection_state_t lora_state;
    ui_connection_state_t ble_state;
    ui_connection_state_t usb_state;
    bool pending_notifications;
} ui_data_t;

/*
 * Initializes sensemate UI (starts a background thread).
 *
 * @param  data_cbs    callbacks used by the UI to read data elements to be displayed.
 * @return 0 on success
 */
int sensemate_ui_init(ui_data_cbs_t *data_cbs);

/*
 * Get the object that holds the UI state.
 * To change something in the UI, update the corresponding value
 * and call @ref sensemate_ui_update()
 *
 * TODO: This pattern sould most likely be replaced by something more
 *       flexible/reactive which only touches affected UI elements.
 *
 * @return pointer to the UI state struct
 */
ui_data_t *sensemate_ui_get_state(void);

/*
 * Update the UI with new values.
 *
 */
void sensemate_ui_update(void);

#endif
