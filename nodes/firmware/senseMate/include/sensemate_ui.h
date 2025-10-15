#ifndef SENSEMATE_UI_H
#define SENSEMATE_UI_H

typedef enum {
    DISCONNECTED,
    ESTABLISHING_CONNECTION,
    CONNECTED,
    RECEIVED,
    TRANSMITTED,
} ui_connection_state_t;

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
 * @return 0 on success
 */
int sensemate_ui_init(void);

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
