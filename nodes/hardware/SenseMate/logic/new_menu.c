#include "new_menu.h"
#include <string.h>
#include <tables.h>
#define MAX_GATES MAX_GATE_COUNT
#define MIN_SIGNAL_STRENGTH 100
#define MIN_JOB_PRIO 1

/*sorted by gate_id*/
gate_entry all_entries [MAX_GATES];
int current_num_gates = 0;

/*sorted by sig_strength, high to low*/
gate_entry *close_by_order [MAX_GATES];
int current_num_close_by = 0;

/*sorted by job_prio high to low*/
gate_entry *jobs_order [MAX_GATES];
int current_num_jobs = 0;


//upper and lower field on Display
display_entry upper_entry;
display_entry lower_entry;


//tables module buffer:
target_state_entry target_state_tbl_entry_buf;
is_state_entry is_state_tbl_entry_buf;
seen_status_entry seen_status_tbl_entry_buf;
jobs_entry jobs_tbl_entry_buf;
timestamp_entry timestamp_tbl_entry_buf;


void in_tables_mark_gate_open_closed(int gate_id, gate_state state){
    //TODO getter to gateTime
    set_seen_status_entry(&(seen_status_entry){
        .gateID = gate_id,
        .status = state,
        .senseMateID = 7,
        .gateTime = 2
    });
};

void update_menu_from_tables(void){

}

void main_menu_entry_view(void){
    upper_entry.menu = MAIN;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = NULL;
    upper_entry.selected = false;

    lower_entry.menu = MAIN;
    lower_entry.subentry = GATES;
    lower_entry.current_gate = NULL;
    lower_entry.selected = true;
}

void init_menue(void){
    main_menu_entry_view();
}

void reorder_close_by(void){
    int added_cnt = 0;
    int curr_sig_strength = 0;
    int last_sig_strenght = INT32_MAX;
    while(added_cnt < current_num_gates && last_sig_strenght > MIN_SIGNAL_STRENGTH){
        //find sigstrenght to add
        for (int i=0; i<current_num_gates; i++){
            if(all_entries[i].sig_strength < last_sig_strenght && all_entries[i].sig_strength > curr_sig_strength){
                curr_sig_strength = all_entries[i].sig_strength;
            }
        }

        //add gates with sigstrengt
        for(int i=0; i<current_num_gates; i++){
            if(all_entries[i].sig_strength == curr_sig_strength){
                close_by_order[added_cnt] = &all_entries[i];
                added_cnt += 1;
            }
        }
        last_sig_strenght = curr_sig_strength;
        curr_sig_strength = 0;
    }
    current_num_close_by = added_cnt;
}


void reorder_jobs(void){
    int added_cnt = 0;
    int curr_prio = 0;
    int last_prio = INT32_MAX;
    while(added_cnt < current_num_gates && last_prio > MIN_JOB_PRIO){
        //find sigstrenght to add
        for (int i=0; i<current_num_gates; i++){
            if(all_entries[i].job_prio < last_prio && all_entries[i].job_prio > curr_prio){
                curr_prio = all_entries[i].job_prio;
            }
        }

        //add gates with sigstrengt
        for(int i=0; i<current_num_gates; i++){
            if(all_entries[i].job_prio == curr_prio){
                jobs_order[added_cnt] = &all_entries[i];
                added_cnt += 1;
            }
        }
        last_prio = curr_prio;
        curr_prio = 0;
    }
    current_num_jobs = added_cnt;
}


void add_gate(gate_entry *new_entry){
    if(current_num_gates >= MAX_GATES){
        return;
    }
    if(new_entry == NULL){
        return;
    }

    //check if gate already there
    for (int i = 0; i < current_num_gates; i++){
        if(all_entries[i].gate_id == new_entry->gate_id){
            return;
        }
    }

    int new_idx = 0;

    for (int i = current_num_gates; i > 0; i--){
        if(all_entries[i].gate_id > new_entry->gate_id){
            all_entries[i+1] = all_entries[i];
        } else{
            new_idx = i;
            break;
        }
    }
    current_num_gates += 1;

    memcpy(&all_entries[new_idx], new_entry, sizeof(gate_entry));
    reorder_close_by();
    reorder_jobs();
}


void rm_gate(int gate_id){
    bool found_to_rm = false;
    for(int i= 0; i<current_num_gates; i++){
        if(found_to_rm && all_entries[i].gate_id > gate_id){
            all_entries[i-1] = all_entries[i];
        }else if(all_entries[i].gate_id == gate_id){
            found_to_rm = true;
        }
    }
    if(found_to_rm){
        current_num_gates -=1;
    }
}


void set_is_state(int gate_id, gate_state state){
    for(int i = 0; i< current_num_gates; i++){
        if(all_entries[i].gate_id == gate_id){
            all_entries[i].gate_is_state = state;
        }
    }
}

void set_requested_state(int gate_id, gate_state state){
    for(int i = 0; i< current_num_gates; i++){
        if(all_entries[i].gate_id == gate_id){
            all_entries[i].gate_requested_state = state;
        }
    }
}

void set_sig_strength(int gate_id, int sig_strenght){
    for(int i = 0; i< current_num_gates; i++){
        if(all_entries[i].gate_id == gate_id){
            all_entries[i].sig_strength = sig_strenght;
        }
    }
    reorder_close_by();
}


void set_job_done(int gate_id, bool done){
    for(int i = 0; i< current_num_gates; i++){
        if(all_entries[i].gate_id == gate_id){
            all_entries[i].job_is_todo = !done;
        }
    }
}


void set_job_prio(int gate_id, int new_prio){
    for(int i = 0; i< current_num_gates; i++){
        if(all_entries[i].gate_id == gate_id){
            all_entries[i].job_prio = new_prio;
        }
    }
    reorder_jobs();
}

int get_num_gates(void){
    return current_num_gates;
}

//get to gate overview menu to header
void main_to_gate_overview(void){
    upper_entry.menu = GATE_OVERVIEW;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = NULL;
    upper_entry.selected = false;

    lower_entry.menu = GATE_OVERVIEW;
    if(current_num_gates > 0){
        lower_entry.subentry = OTHER;
        lower_entry.current_gate = &all_entries[0];
    }else{
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    }
    lower_entry.selected = true;
}

//get back to gate overview menu to the before selected gate
void cancel_to_gate_overview(void){
    upper_entry.menu = GATE_OVERVIEW;
    upper_entry.subentry = OTHER;
    upper_entry.selected = true;

    lower_entry.menu = GATE_OVERVIEW;

    if(&all_entries[current_num_gates -1] == upper_entry.current_gate){
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    }else{
        lower_entry.subentry = OTHER;
        lower_entry.current_gate = &all_entries[current_num_gates -1];
    }
    lower_entry.selected = false;
}

//get to job prios menu to header
void main_to_job_prios(void){
    upper_entry.menu = JOB_PRIOS;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = NULL;
    upper_entry.selected = false;

    lower_entry.menu = JOB_PRIOS;
    if(current_num_jobs > 0){
        lower_entry.subentry = OTHER;
        lower_entry.current_gate = jobs_order[0];
    }else{
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    }
    lower_entry.selected = true;
}

//get back to job prios menu to the before selected gate
void cancel_to_job_prios(void){
    upper_entry.menu = JOB_PRIOS;
    upper_entry.subentry = OTHER;
    upper_entry.selected = true;

    lower_entry.menu = JOB_PRIOS;

    if(jobs_order[current_num_jobs -1] == upper_entry.current_gate){
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    
    }else{
        lower_entry.subentry = OTHER;
        for(int i = 0; i < current_num_jobs; i++){
            if(jobs_order[i] == upper_entry.current_gate){
                lower_entry.current_gate = jobs_order[i+1];
                break;
            }
        }
    }
    lower_entry.selected = false;
}

//get to close by menu to header
void main_to_close_by(void){
    upper_entry.menu = CLOSE_BY_MENU;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = NULL;
    upper_entry.selected = false;

    lower_entry.menu = CLOSE_BY_MENU;
    if(current_num_close_by > 0){
        lower_entry.subentry = OTHER;
        lower_entry.current_gate = jobs_order[0];
    }else{
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    }
    lower_entry.selected = true;
}

//get back to close by menu to the before selected gate
void cancel_to_close_by(void){
    upper_entry.menu = CLOSE_BY_MENU;
    upper_entry.subentry = OTHER;
    upper_entry.selected = true;

    lower_entry.menu = CLOSE_BY_MENU;

    if(close_by_order[current_num_close_by-1] == upper_entry.current_gate){
        lower_entry.subentry = CANCEL;
        lower_entry.current_gate = NULL;
    
    }else{
        lower_entry.subentry = OTHER;
        for(int i = 0; i < current_num_close_by; i++){
            if(close_by_order[i] == upper_entry.current_gate){
                lower_entry.current_gate = close_by_order[i+1];
                break;
            }
        }
    }
    lower_entry.selected = false;
}

void main_menu_input(input input){
    switch (input){
        case UP:
            if(upper_entry.subentry == HEADER){
                return;
            }

            if(upper_entry.selected){
                lower_entry.subentry = upper_entry.subentry;
                upper_entry.subentry -= 1;
            }else{
                upper_entry.selected = true;
                lower_entry.selected = false;
            }

            /*Header not selectable*/
            if(upper_entry.subentry == HEADER){
                upper_entry.selected = false;
                lower_entry.selected = true;
            }
        break;

        case DOWN:
            if(lower_entry.subentry == CLOSE_BY && lower_entry.selected){
                return;
            }
            if(lower_entry.selected){
                upper_entry.subentry = lower_entry.subentry;
                lower_entry.subentry += 1;
            }else{
                upper_entry.selected = false;
                lower_entry.selected = true;
            }

        break;
        
        case SELECT:
            if(upper_entry.selected){
                if(upper_entry.subentry == GATES){
                    main_to_gate_overview();
                }else if (upper_entry.subentry == JOBS){
                    main_to_job_prios();
                }else if (upper_entry.subentry == CLOSE_BY){
                    main_to_close_by();
                }
            }else{
                if(lower_entry.subentry == GATES){
                    main_to_gate_overview();
                }else if (lower_entry.subentry == JOBS){
                    main_to_job_prios();
                }else if (lower_entry.subentry == CLOSE_BY){
                    main_to_close_by();
                }
            }
        break;
    }
}


void specific_gate_selected(gate_entry *gate){
    upper_entry.menu = SELECTED_GATE;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = gate;
    upper_entry.selected = false;

    lower_entry.menu = SELECTED_GATE;
    lower_entry.subentry = MARK_OPEN;
    lower_entry.current_gate = gate;
    lower_entry.selected = true;
}

void gate_overview_input(input input){
    switch(input){
        case UP:
            if(upper_entry.subentry == HEADER){
                return;
            }

            if(lower_entry.selected){
                upper_entry.selected = true;
                lower_entry.selected = false;

            }else if(upper_entry.current_gate == &all_entries[0] && upper_entry.selected){
                upper_entry.subentry = HEADER;
                upper_entry.current_gate = NULL;
                upper_entry.selected = false;

                lower_entry.subentry = OTHER;
                lower_entry.current_gate = &all_entries[0];
                lower_entry.selected = true;

            }else{
                lower_entry.current_gate = upper_entry.current_gate;
                lower_entry.subentry = upper_entry.subentry;
                upper_entry.current_gate -= 1;
                upper_entry.subentry = OTHER;
            }
        break;
        
        case DOWN:
            if(lower_entry.subentry == CANCEL && lower_entry.selected){
                return;
            }

            if(upper_entry.selected){
                lower_entry.selected = true;
                upper_entry.selected = false;
            
            }else if(lower_entry.current_gate == &all_entries[current_num_gates - 1] && lower_entry.selected){
                lower_entry.subentry = CANCEL;
                lower_entry.current_gate = NULL;

                upper_entry.subentry = OTHER;
                upper_entry.current_gate = &all_entries[current_num_gates - 1];
                
            }else{
                upper_entry.current_gate = lower_entry.current_gate;
                upper_entry.subentry = lower_entry.subentry;
                lower_entry.current_gate += 1;
                lower_entry.subentry = OTHER;
            }
        break;

        case SELECT:
            if(upper_entry.selected && upper_entry.subentry == OTHER){
                specific_gate_selected(upper_entry.current_gate);
            }else if (lower_entry.selected && lower_entry.subentry == CANCEL){
                main_menu_entry_view();
            }else if (lower_entry.selected && lower_entry.subentry == OTHER){
                specific_gate_selected(lower_entry.current_gate);
            }
        break;

        default:
        break;
    }
}


void specific_job_selected(gate_entry *gate){
    upper_entry.menu = SELECTED_JOB;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = gate;
    upper_entry.selected = false;

    lower_entry.menu = SELECTED_JOB;
    lower_entry.subentry = MARK_OPEN;
    lower_entry.current_gate = gate;
    lower_entry.selected = true;
}

void job_prios_input(input input){
    switch (input){
    case UP:
        if(upper_entry.subentry == HEADER){
            return;
        }

        if(lower_entry.selected){
            upper_entry.selected = true;
            lower_entry.selected = false;
        }else if(upper_entry.current_gate == jobs_order[0] && upper_entry.selected){
            upper_entry.subentry = HEADER;
            upper_entry.current_gate = NULL;
            upper_entry.selected = false;
            lower_entry.subentry = OTHER;
            lower_entry.current_gate = jobs_order[0];
            lower_entry.selected = true;
        }else{
            lower_entry.current_gate = upper_entry.current_gate;
            lower_entry.subentry = upper_entry.subentry;
            for(int i = 0; i < current_num_jobs; i++){
                if(jobs_order[i] == upper_entry.current_gate){
                    upper_entry.current_gate = jobs_order[i-1];
                    break;
                }
            }
            upper_entry.subentry = OTHER;
        }
    break;
    
    case DOWN:
        if(lower_entry.subentry == CANCEL && lower_entry.selected){
            return;
        }

        if(upper_entry.selected){
            lower_entry.selected = true;
            upper_entry.selected = false;
        }else if(lower_entry.current_gate == jobs_order[current_num_jobs -1] && lower_entry.selected){
            lower_entry.subentry = CANCEL;
            lower_entry.current_gate = NULL;

            upper_entry.subentry = OTHER;
            upper_entry.current_gate = jobs_order[current_num_jobs - 1];
        }else{
            upper_entry.current_gate = lower_entry.current_gate;
            upper_entry.subentry = lower_entry.subentry;
            for(int i = 0; i < current_num_jobs; i++){
                if(jobs_order[i] == lower_entry.current_gate){
                    lower_entry.current_gate = jobs_order[i+1];
                    break;
                }
            }
            lower_entry.subentry = OTHER;
        }
    break;

    case SELECT:
        if(upper_entry.selected && upper_entry.subentry == OTHER){
            specific_job_selected(upper_entry.current_gate);
        }
        else if (lower_entry.selected && lower_entry.subentry == CANCEL){
            main_menu_entry_view();
        }else if (lower_entry.selected && lower_entry.subentry == OTHER){
            specific_job_selected(lower_entry.current_gate);
        }
    default:
        break;
    }
}


void specific_close_by_selected(gate_entry *gate){
    upper_entry.menu = SELECTED_CLOSE_BY;
    upper_entry.subentry = HEADER;
    upper_entry.current_gate = gate;
    upper_entry.selected = false;

    lower_entry.menu = SELECTED_CLOSE_BY;
    lower_entry.subentry = MARK_OPEN;
    lower_entry.current_gate = gate;
    lower_entry.selected = true;
}

void close_by_input(input input){
    switch (input){
    case UP:
        if(upper_entry.subentry == HEADER){
            return;
        }

        if(lower_entry.selected){
            upper_entry.selected = true;
            lower_entry.selected = false;
        }else if(upper_entry.current_gate == close_by_order[0] && upper_entry.selected){
            upper_entry.subentry = HEADER;
            upper_entry.current_gate = NULL;
            upper_entry.selected = false;
            lower_entry.subentry = OTHER;
            lower_entry.current_gate = close_by_order[0];
            lower_entry.selected = true;
        }else{
            lower_entry.current_gate = upper_entry.current_gate;
            lower_entry.subentry = upper_entry.subentry;
            for(int i = 0; i < current_num_close_by; i++){
                if(close_by_order[i] == upper_entry.current_gate){
                    upper_entry.current_gate = close_by_order[i-1];
                    break;
                }
            }
            upper_entry.subentry = OTHER;
        }
    break;
    
    case DOWN:
        if(lower_entry.subentry == CANCEL && lower_entry.selected){
            return;
        }

        if(upper_entry.selected){
            lower_entry.selected = true;
            upper_entry.selected = false;
        }else if(lower_entry.current_gate == close_by_order[current_num_close_by -1] && lower_entry.selected){
            lower_entry.subentry = CANCEL;
            lower_entry.current_gate = NULL;

            upper_entry.subentry = OTHER;
            upper_entry.current_gate = close_by_order[current_num_close_by - 1];
        }else{
            upper_entry.current_gate = lower_entry.current_gate;
            upper_entry.subentry = lower_entry.subentry;
            for(int i = 0; i < current_num_close_by; i++){
                if(close_by_order[i] == lower_entry.current_gate){
                    lower_entry.current_gate = close_by_order[i+1];
                    break;
                }
            }
            lower_entry.subentry = OTHER;
        }
    break;

    case SELECT:
        if(upper_entry.selected && upper_entry.subentry == OTHER){
            specific_close_by_selected(upper_entry.current_gate);
        }
        else if (lower_entry.selected && lower_entry.subentry == CANCEL){
            main_menu_entry_view();
        }else if (lower_entry.selected && lower_entry.subentry == OTHER){
            specific_close_by_selected(lower_entry.current_gate);
        }
    default:
        break;
    }
}

void to_confirmation_open(menu_type menu){
    upper_entry.selected = false;
    lower_entry.selected = true;
    upper_entry.subentry = HEADER;
    lower_entry.subentry = CONFIRM;
    switch (menu){
        case SELECTED_GATE:
            upper_entry.menu = CONFIRMATION_GATE_OPEN;
            lower_entry.menu = CONFIRMATION_GATE_OPEN;
        break;

        case SELECTED_JOB:
            upper_entry.menu = CONFIRMATION_JOB_OPEN;
            lower_entry.menu = CONFIRMATION_JOB_OPEN;
        break;

        case SELECTED_CLOSE_BY:
            upper_entry.menu = CONFIRMATION_CLOSE_BY_OPEN;
            lower_entry.menu = CONFIRMATION_CLOSE_BY_OPEN;
        break;

        default:
        break;
    }
}

void to_confirmation_closed(menu_type menu){
    upper_entry.selected = false;
    lower_entry.selected = true;
    upper_entry.subentry = HEADER;
    lower_entry.subentry = CONFIRM;
    switch (menu){
        case SELECTED_GATE:
            upper_entry.menu = CONFIRMATION_GATE_CLOSE;
            lower_entry.menu = CONFIRMATION_GATE_CLOSE;
        break;

        case SELECTED_JOB:
            upper_entry.menu = CONFIRMATION_JOB_CLOSE;
            lower_entry.menu = CONFIRMATION_JOB_CLOSE;
        break;

        case SELECTED_CLOSE_BY:
            upper_entry.menu = CONFIRMATION_CLOSE_BY_CLOSE;
            lower_entry.menu = CONFIRMATION_CLOSE_BY_CLOSE;
        break;

        default:
        break;
    }
}

void selected_input(input input, menu_type menu){
    switch (input){
        case UP:
            if(upper_entry.subentry == HEADER){
                return;
            }

            if(lower_entry.selected){
                upper_entry.selected = true;
                lower_entry.selected = false;
            } else if(upper_entry.selected && upper_entry.subentry == MARK_OPEN){
                upper_entry.subentry = HEADER;
                upper_entry.selected = false;
                lower_entry.subentry = MARK_OPEN;
                lower_entry.selected = true;
            }else{
                lower_entry.subentry = upper_entry.subentry;
                upper_entry.subentry -= 1;
            }
        break;

        case DOWN:
            if(lower_entry.subentry == CANCEL && lower_entry.selected){
                return;
            }

            if(upper_entry.selected){
                lower_entry.selected = true;
                upper_entry.selected = false;
            }else{
                upper_entry.subentry = lower_entry.subentry;
                lower_entry.subentry += 1;
            }
        break;

        case SELECT:
            if(upper_entry.selected){
                if(upper_entry.subentry == MARK_OPEN){
                    to_confirmation_open(menu);

                }else if (upper_entry.subentry == MARK_CLOSED){
                    to_confirmation_closed(menu);

                }else if (upper_entry.subentry == CANCEL){
                    if(menu == SELECTED_GATE){
                        cancel_to_gate_overview();
                        
                    }else if (menu == SELECTED_JOB){
                        cancel_to_job_prios();

                    }else if (menu == SELECTED_CLOSE_BY){
                        cancel_to_close_by();
                    }
                }
            }else{

            }
    }
}


void confirmation_open_closed(input input, menu_type menu, gate_state state){
    switch (input){
        case UP:
            if(upper_entry.subentry == HEADER){
                return;
            }

            if(lower_entry.selected){
                upper_entry.selected = true;
                lower_entry.selected = false;
            }else if (upper_entry.subentry == CONFIRM){
                lower_entry.subentry = upper_entry.subentry;
                upper_entry.subentry = HEADER;
            }
        break;

        case DOWN:
            if(lower_entry.subentry == CANCEL && lower_entry.selected){
                return;
            }

            if(upper_entry.selected){
                lower_entry.selected = true;
                upper_entry.selected = false;
            }else if (lower_entry.subentry == CONFIRM){
                upper_entry.subentry = lower_entry.subentry;
                lower_entry.subentry = CANCEL;
            }
        break;
            
        case SELECT:
            if(upper_entry.selected && upper_entry.subentry == CONFIRM){
                
                if(state == OPEN){
                    set_requested_state(upper_entry.current_gate->gate_id, OPEN);
                }else if (state == CLOSED){
                    set_requested_state(upper_entry.current_gate->gate_id, CLOSED);
                }
                
                if(upper_entry.current_gate->gate_is_state == upper_entry.current_gate->gate_requested_state){
                    set_job_done(upper_entry.current_gate->gate_id, true);
                }
            } else if (lower_entry.selected && lower_entry.subentry == CONFIRM){

                if(state == OPEN){
                    set_requested_state(lower_entry.current_gate->gate_id, OPEN);
                }else if (state == CLOSED){
                    set_requested_state(lower_entry.current_gate->gate_id, CLOSED);
                }
                
                if(lower_entry.current_gate->gate_is_state == lower_entry.current_gate->gate_requested_state){
                    set_job_done(lower_entry.current_gate->gate_id, true);
                }
            }

            if(menu == CONFIRMATION_GATE_OPEN){
                cancel_to_gate_overview();
            }else if (menu == CONFIRMATION_JOB_OPEN){
                cancel_to_job_prios();
            }else if (menu == CONFIRMATION_CLOSE_BY_OPEN){
                cancel_to_close_by();
            }
        break; 
    }
}



void menu_input(input input){
    switch (upper_entry.menu){
        case MAIN:
            main_menu_input(input);
        break;

        case GATE_OVERVIEW:
            gate_overview_input(input);
        break;

        case JOB_PRIOS:
            job_prios_input(input);
        break;

        case CLOSE_BY_MENU:
            close_by_input(input);
        break;

        case SELECTED_GATE:
            selected_input(input, SELECTED_GATE);
        break;

        case CONFIRMATION_GATE_OPEN:
            confirmation_open_closed(input, CONFIRMATION_GATE_OPEN, OPEN);
        break;

        case CONFIRMATION_GATE_CLOSE:
            confirmation_open_closed(input, CONFIRMATION_GATE_CLOSE, CLOSED);
        break;

        case SELECTED_JOB:
            selected_input(input, SELECTED_JOB);
        break;

        case CONFIRMATION_JOB_OPEN:
            confirmation_open_closed(input, CONFIRMATION_JOB_OPEN, OPEN);
        break;

        case CONFIRMATION_JOB_CLOSE:
            confirmation_open_closed(input, CONFIRMATION_JOB_CLOSE, CLOSED);
        break;

        case SELECTED_CLOSE_BY:
            selected_input(input, SELECTED_CLOSE_BY);
        break;

        case CONFIRMATION_CLOSE_BY_OPEN:
            confirmation_open_closed(input, CONFIRMATION_CLOSE_BY_OPEN, OPEN);
        break;

        case CONFIRMATION_CLOSE_BY_CLOSE:
            confirmation_open_closed(input, CONFIRMATION_CLOSE_BY_CLOSE, CLOSED);
        break;

        default:
        break;
    }
    
}

