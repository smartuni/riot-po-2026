#ifndef UI_H
#define UI_H

typedef struct {
    uint16_t visible_gate_cnt;
    uint16_t pending_jobs_cnt;
    uint16_t visible_mate_cnt;
} ui_data_t;

/*
 * Initializes sensemate UI (starts a background thread).
 *
 * @return 0 on success
 */
int sensemate_ui_init(void);

/* 
 * update the UI with new values.
 *
 */
void sensemate_ui_update(ui_data_t *data);

#endif
