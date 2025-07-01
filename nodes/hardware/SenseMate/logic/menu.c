#include <stdio.h>
#include "menu.h"
#include "displayDemo.h"
#include <stddef.h>
#include <string.h>
#include "u8g2.h"
#include "tables.h"
#include "mate_lorawan.h"

#define GATE "Gate "
#define CANCEL "cancel"
#define MAX_GATES 15

#define OPEN_GATE_STATE 0
#define JOB_STATE 1

#define CLOSED 0
#define OPEN 1

#define SENSEMATE_ID 7

#define TABLE_SUCCESS           0
#define TABLE_ERROR_SIZE_TOO_BIG -1
#define TABLE_ERROR_INVALID_GATE_ID -2
#define TABLE_ERROR_NOT_FOUND   -3


menu_entry_t set_job_state[3];
menu_entry_t jobs[MAX_GATES+1];
menu_entry_t main_menu [2];
menu_entry_t gate_states[MAX_GATES+1];
menu_entry_t set_gate[3];
menu_entry_t confirm_gate[2];

menu_header_t set_jobs_state_header;
menu_header_t jobs_header;
menu_header_t main_menu_header;
menu_header_t gate_states_header;
menu_header_t set_gate_header;
menu_header_t confirm_gate_header;

menu_header_t *active_menu = &main_menu_header;
menu_entry_t *active_entry = main_menu;
int active_entry_index = 0;
menu_entry_t *other_entry = NULL;
int other_entry_index = -1;
int new_gate_state = CLOSED; // Default state for new gates

int active_job = -1;
int active_gate = -1;

target_state_entry  target_state_buf;
is_state_entry is_state_buf;
seen_status_entry seen_status_buf;
jobs_entry jobs_buf;

int my_timestamp = 1;

void init_menu(void){

    set_job_state[0] = (menu_entry_t){"Done", -1, 0, &jobs_header};
    set_job_state[1] = (menu_entry_t){"Not done", -1, 0, &jobs_header};
    set_job_state[2] = (menu_entry_t){CANCEL, -1, 0, &jobs_header};

    main_menu[0] = (menu_entry_t){"Gate status", -1, 0, &gate_states_header};
    main_menu[1] = (menu_entry_t){"Jobs", -1, 0, &jobs_header};

    set_gate[0] = (menu_entry_t){"set open", -1, 0, &confirm_gate_header};
    set_gate[1] = (menu_entry_t){"set closed", -1, 0, &confirm_gate_header};
    set_gate[2] = (menu_entry_t){CANCEL, -1, 0, &gate_states_header};

    confirm_gate[0] = (menu_entry_t){"confirm", -1, 0, &gate_states_header};
    confirm_gate[1] = (menu_entry_t){CANCEL, -1, 0, &set_gate_header};

    set_jobs_state_header = (menu_header_t){
        .text = "Job -Gate ",
        .id = -1,
        .first_entry = set_job_state,
        .num_entries = 3
    };

    jobs_header = (menu_header_t){
        .text = "Jobs",
        .id = -1,
        .first_entry = jobs,
        .num_entries = 0
    };

    main_menu_header = (menu_header_t){
        .text = "Main Menu",
        .id = -1,
        .first_entry = main_menu,
        .num_entries = 2
    };

    gate_states_header = (menu_header_t){
        .text = "Gate states",
        .id = -1,
        .first_entry = gate_states,
        .num_entries = 0
    };

    set_gate_header = (menu_header_t){
        .text = "set Gate",
        .id = -1,
        .first_entry = set_gate,
        .num_entries = 3
    };

    confirm_gate_header = (menu_header_t){
        .text = "confirm Gate",
        .id = -1,
        .first_entry = confirm_gate,
        .num_entries = 2
    };

    int initial_ts = 1;
    uint8_t initstate = 0;
    uint8_t senseMateID = SENSEMATE_ID;
    
    target_state_entry  initial_target_state = {
        .gateID = 0,
        .state = initstate,
        .timestamp = initial_ts
    };

    is_state_entry initial_is_state = {
        .gateID = 0,
        .state = initstate,
        .gateTime = initial_ts
    };

    seen_status_entry initial_seen_status = {
        .gateID = 0,
        .gateTime = initial_ts,
        .status = initstate,
        .senseMateID = senseMateID
    };

    for (int i = 0; i < MAX_GATES; i++) {
        gate_states[i].next = &set_gate_header;
        gate_states[i].id = i+1;
        gate_states[i].text = GATE;
        gate_states[i].state = 0;
        gate_states_header.num_entries++;
        
        jobs[i].next = &set_jobs_state_header;
        jobs[i].id = i+1;
        jobs[i].text = GATE;
        jobs[i].state = 0;

        initial_target_state.gateID = i;
        initial_is_state.gateID = i;
        initial_seen_status.gateID = i;
        set_target_state_entry(&initial_target_state);
        set_is_state_entry(&initial_is_state);
        set_seen_status_entry(&initial_seen_status);
    }

    gate_states[MAX_GATES].next = &main_menu_header; // Last gate points to main menu
    gate_states[MAX_GATES].text = CANCEL; // Last gate is the cancel option
    gate_states[MAX_GATES].id = -1; // Last gate has no ID
    gate_states[MAX_GATES].state = 0; // Last gate has no state
    gate_states_header.num_entries++;

    jobs[0].next = &main_menu_header; // Last job points to main menu
    jobs[0].text = CANCEL; // First job is the cancel option
    jobs[0].id = -1; // First job has no ID
    jobs[0].state = 0; // First job has no state
    jobs_header.num_entries = 1; // Start with the cancel job
    add_job(1, OPEN); // Add the cancel job
}

void add_job(int id, uint8_t expected_state) {
    if (id < 1 || id > MAX_GATES) {
        return; // Invalid job ID
    }
    if (jobs_header.num_entries <= MAX_GATES) {
        jobs[jobs_header.num_entries] = jobs[jobs_header.num_entries-1]; // Copy the last entry (cancel) to the next position
        jobs[jobs_header.num_entries-1].id = id;
        jobs[jobs_header.num_entries-1].state = 0;
        jobs[jobs_header.num_entries-1].state |= expected_state;
        jobs[jobs_header.num_entries-1].state |= JOB_IN_PROGRESS;
        jobs_header.num_entries++;

        gate_states[id -1].state |= JOB_IN_PROGRESS;
    }
}

void mark_job_done(int id) {
    if (id < 0 || id > MAX_GATES) {
        return; // Invalid job ID
    }
    
    for (int i = 0; i < jobs_header.num_entries; i++) {
        if (jobs[i].id == id) {
            jobs[i].state &= ~JOB_IN_PROGRESS; // Clear the JOB_IN_PROGRESS bit
            gate_states[id - 1].state &= ~JOB_IN_PROGRESS; // Clear the JOB_IN_PROGRESS bit in gate state
            break;
        }
    }
}

void mark_job_todo(int id) {
    if (id < 1 || id > MAX_GATES) {
        return; // Invalid job ID
    }
    
    for (int i = 0; i < jobs_header.num_entries; i++) {
        if (jobs[i].id == id) {
            jobs[i].state |= JOB_IN_PROGRESS; // Set the JOB_IN_PROGRESS bit
            gate_states[id - 1].state |= JOB_IN_PROGRESS; // Set the JOB_IN_PROGRESS bit in gate state
            break;
        }
    }
}

void set_gate_state(int id, uint8_t state) {
    if (id < 1 || id > MAX_GATES) {
        return; // Invalid gate ID
    }
    
    gate_states[id - 1].state = state;
}

void mark_gate_open(int id) {
    gate_states[id - 1].state |= OPEN; // Set the OPEN bit
}

void mark_gate_closed(int id) {
    gate_states[id - 1].state &= ~OPEN; // Clear the OPEN bit
}

void set_current_meustate(int input) {
    switch (input)
    {
    case INIT:
        active_menu = &main_menu_header;
        active_entry = main_menu;
        other_entry = NULL;
        active_entry_index = 0;
        other_entry_index = -1;
        active_job = -1;
        active_gate = -1;
        break;

    case UP: // UP
        if (active_entry_index > 0) {
            other_entry_index = active_entry_index;
            other_entry = active_entry;
            active_entry_index--;
            active_entry = &active_menu->first_entry[active_entry_index]; 
            if(active_menu == &jobs_header){
                active_job --;
            }
            if(active_menu == &gate_states_header){
                active_gate --;
            }
        } else if (other_entry_index >= 0) {
            other_entry = NULL;
            other_entry_index = -1;
        }
        break;
    case DOWN: // DOWN
        if (active_entry_index < active_menu->num_entries - 1) {
            other_entry_index = active_entry_index;
            other_entry = active_entry;
            active_entry_index++;
            active_entry = &active_menu->first_entry[active_entry_index];
            if(active_menu == &jobs_header && active_job < jobs_header.num_entries - 2){ // -2 because last entry is cancel
                active_job ++;
            }
            if(active_menu == &gate_states_header && active_gate < gate_states_header.num_entries - 2){ // -2 because last entry is cancel
                active_gate ++;
            }
        }
        break;

    case SELECT: // SELECT
        if(active_entry->next == NULL){
            set_current_meustate(INIT); // Reset to main menu if no next menu
        } else {
            //operation of closing / opening a gate or marking jobs before changing to the next menu
            
            if (active_menu == &confirm_gate_header) { // If we are in the confirm gate menu
                if (active_entry_index == 0) { //CONFIRM
                    get_seen_status_entry(active_gate + 1, &seen_status_buf);
                    get_is_state_entry(active_gate + 1, &is_state_buf);
                    get_target_state_entry(active_gate + 1, &target_state_buf);

                    int resu = TABLE_ERROR_NOT_FOUND;

                    if (new_gate_state == OPEN) {
                        
                        mark_gate_open(active_gate + 1); // Mark the gate as open
                        resu = set_seen_status_entry(&(seen_status_entry){
                            .gateID = active_gate,
                            .status = OPEN,
                            .senseMateID = SENSEMATE_ID,
                            .gateTime = my_timestamp++
                        });
                    } else if (new_gate_state == CLOSED) {
                        mark_gate_closed(active_gate + 1); // Mark the gate as closed
                        resu = set_seen_status_entry(&(seen_status_entry){
                            .gateID = active_gate,
                            .status = CLOSED,
                            .senseMateID = SENSEMATE_ID,
                            .gateTime = my_timestamp++
                        });
                    }

                    if(resu == TABLE_SUCCESS){
                        //send_event
                        printf("send_event \n");
                        event_post(&lorawan_queue, &send_event); // Post the send event to the lorawan queue
                    }


                }
            } else if (active_menu == &set_gate_header){
                if (active_entry_index == 0) { // OPEN
                    new_gate_state = OPEN;
                } else if (active_entry_index == 1) { // CLOSED
                    new_gate_state = CLOSED;
                }

            } else if (active_menu == &set_jobs_state_header){
                if (active_entry_index != 2) { // not cancel
                    get_jobs_entry(jobs[active_job].id, &jobs_buf);

                    int resu = TABLE_ERROR_NOT_FOUND;

                    if (active_entry_index == 0) { //Done
                        
                        mark_job_done(jobs[active_job].id); // Mark the job as done in UI
                        jobs_buf.done = JOB_DONE;
                        resu = set_jobs_entry(&jobs_buf); // Update the job entry in the table

                    } else if (active_entry_index == 1) { //Not done
                        mark_job_todo(jobs[active_job].id); // Mark the job as done in UI
                        jobs_buf.done = 1;
                        resu = set_jobs_entry(&jobs_buf); // Update the job entry in the table
                    }

                    if(resu == TABLE_SUCCESS){
                        //send_event
                        printf("job marked done \n");
                        
                    }


                }
            }

            active_menu = active_entry->next; // Move to the next menu

            if (active_menu == &jobs_header) {
                if  (active_job != -1){
                    // If we are in the jobs menu and a job is selected
                active_entry = &active_menu->first_entry[active_job]; // Set the active entry to the current job
                active_entry_index = active_job; // Set the active entry index to the current job
                other_entry = &active_menu->first_entry[active_job+1]; // Set the other entry to the next job
                other_entry_index = active_job +1;
                }else{
                    // If we are in the jobs menu and no job is selected
                    active_entry = &active_menu->first_entry[0]; // Set the active entry to the first job or cancel
                    active_entry_index = 0; // Set the active entry index to 0
                    other_entry = NULL; // No other entry
                    other_entry_index = -1; // No other entry index
                }

            } else if (active_menu == &gate_states_header) {
                if (active_gate != -1){
                    // If we are in the gate states menu and a gate is selected
                    active_entry = &active_menu->first_entry[active_gate]; // Set the active entry to the current gate
                    active_entry_index = active_gate; // Set the active entry index to the current gate
                    other_entry = &active_menu->first_entry[active_gate+1]; // Set the other entry to the next gate
                    other_entry_index = active_gate + 1; // Set the other entry index to the next gate
                } else {
                    // If we are in the gate states menu and no gate is selected
                    active_entry = &active_menu->first_entry[0]; // Set the active entry to the first gate
                    active_entry_index = 0; // Set the active entry index to 0
                    other_entry = NULL; // No other entry
                    other_entry_index = -1; // No other entry index
                }
            } else {
                active_entry = active_menu->first_entry; // Set the active entry to the first entry of the new menu
                active_entry_index = 0; // Reset the active entry index
                other_entry = NULL; // No other entry
                other_entry_index = -1; // No other entry index
            }
        }
        break;

    default:
        break;
    }
}

void refresh_menu(void) {
    for (int i = 0; i < gate_states_header.num_entries; i++) {
        get_is_state_entry(i, &is_state_buf);
        get_seen_status_entry(i, &seen_status_buf);
        if(is_state_buf.gateTime  >  seen_status_buf.gateTime){
            gate_states[i].state = is_state_buf.state;
        }else{
            gate_states[i].state = seen_status_buf.status;
        }
    }
}

void refresh_display(void) {
    new_page(); // Clear the display for the new page
    
    if (active_menu == &jobs_header) {
        if (other_entry != NULL) {
            if(active_entry_index < other_entry_index){

                display_job_menu_box(active_entry->text, active_entry->id, true, true, active_entry->state & JOB_TODO, active_entry->state & OPEN, active_entry_index > 0);
                printf("give number %d\n", active_entry->id);

                if(other_entry_index < active_menu->num_entries - 1){
                    display_job_menu_box(other_entry->text, other_entry->id, false, false, other_entry->state & JOB_TODO, active_entry->state & OPEN, active_menu->num_entries > other_entry_index + 1);
                    printf("give number %d\n", other_entry->id);
                }else{
                    display_ordinary_menu(other_entry->text, 0, false, false, false, active_menu->num_entries > other_entry_index + 1);
                }

            } else {
                
                display_job_menu_box(other_entry->text, other_entry->id, true, false, other_entry->state & JOB_TODO, active_entry->state & OPEN, other_entry_index > 0);
                printf("give number %d\n", other_entry->id);

                if(active_entry_index < active_menu->num_entries - 1){
                    display_job_menu_box(active_entry->text, active_entry->id, false, true, active_entry->state & JOB_TODO, active_entry->state & OPEN, active_menu->num_entries > active_entry_index + 1);
                    printf("give number %d\n", active_entry->id);
                }else{
                    display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
                }
            }

        } else {
            display_menu_header(active_menu->text, 0, false);
            if(active_entry_index < active_menu->num_entries - 1){
                display_job_menu_box(active_entry->text, active_entry->id, false, true, active_entry->state & JOB_TODO, active_entry->state & OPEN, active_menu->num_entries > active_entry_index + 1);
                printf("give number %d\n", active_entry->id);
            }else{
                display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
            }
        }

        
    } else if (active_menu == &gate_states_header) {
        if (other_entry != NULL) {
            if(active_entry_index < other_entry_index){

                display_gate_menu_box(active_entry->text, active_entry->id, true, true, active_entry->state & OPEN, active_entry_index > 0);
                printf("give number %d\n", active_entry->id);

                if(other_entry_index < active_menu->num_entries - 1){
                    display_gate_menu_box(other_entry->text, other_entry->id, false, false, other_entry->state & OPEN, active_menu->num_entries > other_entry_index + 1);
                    printf("give number %d\n", other_entry->id);
                }else{
                    display_ordinary_menu(other_entry->text, 0, false, false, false, active_menu->num_entries > other_entry_index + 1);
                }

            } else {
                
                display_gate_menu_box(other_entry->text, other_entry->id, true, false, other_entry->state & OPEN, other_entry_index > 0);
                printf("give number %d\n", other_entry->id);

                if(active_entry_index < active_menu->num_entries - 1){
                    display_gate_menu_box(active_entry->text, active_entry->id, false, true, active_entry->state & OPEN, active_menu->num_entries > active_entry_index + 1);
                    printf("give number %d\n", active_entry->id);
                }else{
                    display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
                }
            }

        } else {
            display_menu_header(active_menu->text, 0, false);
            if(active_entry_index < active_menu->num_entries - 1){
                display_gate_menu_box(active_entry->text, active_entry->id, false, true, active_entry->state & OPEN, active_menu->num_entries > active_entry_index + 1);
                printf("give number %d\n", active_entry->id);
            }else{
                display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
            }
        }
    } else{
        if (other_entry != NULL) {
            if(active_entry_index < other_entry_index){
                display_ordinary_menu(active_entry->text, 0, false, true, true, active_entry_index > 0);
                display_ordinary_menu(other_entry->text, 0, false, false, false, active_menu->num_entries > other_entry_index + 1);
            } else {
                display_ordinary_menu(other_entry->text, 0, false, true, false, other_entry_index > 0);
                display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
            }
        } else {
            if(active_menu == &set_gate_header || active_menu == &confirm_gate_header){
                display_menu_header(active_menu->text, active_gate+1, true);
            }else{
                display_menu_header(active_menu->text, 0, false);
            }
            
            display_ordinary_menu(active_entry->text, 0, false, false, true, active_menu->num_entries > active_entry_index + 1);
        }
    }

    
    
}