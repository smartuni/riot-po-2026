#ifndef _NEW_MENU_H
#define _NEW_MENU_H

#include <stdbool.h>
#include <stdint.h>


typedef enum{
    UP,
    DOWN,
    SELECT
} input;


typedef enum{
    CLOSED = 0,
    OPEN,
    UNKNOWN
} gate_state;

typedef enum {
    MAIN,
    GATE_OVERVIEW,
    JOB_PRIOS,
    CLOSE_BY_MENU,
    SELECTED_GATE,
    CONFIRMATION_GATE_OPEN,
    CONFIRMATION_GATE_CLOSE,
    SELECTED_JOB,
    CONFIRMATION_JOB_OPEN,
    CONFIRMATION_JOB_CLOSE,
    SELECTED_CLOSE_BY,
    CONFIRMATION_CLOSE_BY_OPEN,
    CONFIRMATION_CLOSE_BY_CLOSE
}menu_type;

/*DO NOT CHANGE ORDER!!!
if there are things to be added, then in the beginnig or end*/
typedef enum {
    MARK_OPEN,
    MARK_CLOSED,
    CANCEL,
    CONFIRM,
    HEADER,
    GATES,
    JOBS,
    CLOSE_BY,
    OTHER
}subentry;

typedef struct {
    uint8_t gate_id;
    uint8_t gate_is_state;
    uint8_t gate_requested_state;
    bool job_is_todo;
    uint8_t job_prio;
    uint8_t sig_strength;
} gate_entry;

typedef struct {
    menu_type menu;
    subentry subentry;
    gate_entry *current_gate;
    bool selected;
} display_entry;

extern void init_menue(void);

extern void add_gate(gate_entry *new_entry);

extern void rm_gate(int gate_id);

extern void set_is_state(int gate_id, gate_state state);

extern void set_requested_state(int gate_id, gate_state state);

extern void set_sig_strength(int gate_id, int sig_strength);

extern void set_job_done(int gate_id, bool done);

extern void set_job_prio(int gate_id, int new_prio);

extern int get_num_gates(void);

extern void menu_input(input input);


#endif
