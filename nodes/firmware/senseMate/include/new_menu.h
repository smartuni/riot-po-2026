#ifndef _NEW_MENU_H
#define _NEW_MENU_H

#include <stdbool.h>
#include <stdint.h>

/*
 * input options for the menu
*/
typedef enum{
    UP,
    DOWN,
    SELECT
} input;

/*
 * gate state options
 * CLOSED = 0, OPEN = 1, UNKNOWN = 2
*/
typedef enum{
    CLOSED = 0,
    OPEN,
    UNKNOWN
} gate_state;

/*
 * menu type options
 * the menu_type is used to determine in which menu the user currently is
 * and which menu should be displayed
*/
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
if there are things to be added, then in the beginnig or end

* subentry is used to determine the current subentry in the menu
* and which subentry should be displayed
*/
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

/*
 * gate entry structure
 */
typedef struct {
    uint8_t gate_id;
    uint8_t gate_is_state;
    uint8_t gate_requested_state;
    bool job_is_todo;
    uint8_t job_prio;
    int8_t sig_strength;
} gate_entry;

/*
 * display entry structure
 * used to store the current menu state and the current gate
 * It's either upper or lower entry
 */
typedef struct {
    menu_type menu;
    subentry subentry;
    gate_entry *current_gate;
    bool selected;
} display_entry;

/*
 * draws the menu on the display
 */
extern void init_new_menu(void);

/*
 * adds a new gate entry to the menu of the display
 * @param new_entry The new gate entry to add
 * If the gate already exists, it will not be added
 * If the maximum number of gates is reached, it will not be added
 * If the new_entry is NULL, it will not be added
 * The gates are sorted by gate_id
 */
extern void add_gate(gate_entry *new_entry);

/*
 * removes a gate entry from the menu of the display
 * @param gate_id The id of the gate to remove
 * If the gate does not exist, nothing will happen
 * The gates stay sorted by gate_id
 */
extern void rm_gate(int gate_id);

/*
 * sets the is_state of a gate entry
 * @param gate_id The id of the gate to set the state for
 * @param state The state to set, either OPEN or CLOSED
 * If the gate does not exist, nothing will happen
 */
extern void set_is_state(int gate_id, gate_state state);

/*
 * sets the requested state of a gate entry
 * @param gate_id The id of the gate to set the requested state for
 * @param state The requested state to set, either OPEN or CLOSED
 * If the gate does not exist, nothing will happen
 */
extern void set_requested_state(int gate_id, gate_state state);

/*
 * sets the signal strength of a gate entry
 * @param gate_id The id of the gate to set the signal strength for
 * @param sig_strength The signal strength to set
 * If the gate does not exist, nothing will happen
 */
extern void set_sig_strength(int gate_id, int sig_strength);

/*
 * sets the job done state of a gate entry
 * @param gate_id The id of the gate to set the job done state for
 * @param done If true, the job is done, if false, the job is not done
 * If the gate does not exist, nothing will happen
 */
extern void set_job_done(int gate_id, bool done);

/*
 * sets the job priority of a gate entry
 * @param gate_id The id of the gate to set the job priority for
 * @param new_prio The new job priority to set
 * If the gate does not exist, nothing will happen
 */
extern void set_job_prio(int gate_id, int new_prio);

/*
 * gets the number of gates in the menu
 * @return The number of gates in the menu
 */
extern int get_num_gates(void);

/*
 * processes user input for the menu
 * @param input The user input to process
 */
extern void menu_input(input input);

/*
 * checks if a gate is listed in the menu
 * @param gate_id The id of the gate to check
 * @return true if the gate is listed, false otherwise
 */
bool gate_listed(int gate_id);


/*
 * updates the menu from the tables
 * This function will read the target state, is state, jobs and timestamp tables
 * and update the menu accordingly
 */
extern void update_menu_from_tables(void);

/*
 * updates the menu display
 * This function will redraw the menu on the display
 */
void update_menu_display(void);
#endif
