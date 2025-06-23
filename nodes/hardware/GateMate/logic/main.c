#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "tables.h"
#include "mate_lorawan.h"

#include <stdio.h>

#include "header/detectDoorStatus.h"
#include "header/event_creation.h"


char lorawan_stack[THREAD_STACKSIZE_DEFAULT];
char ble_stack[THREAD_STACKSIZE_DEFAULT];

int main(void){
    init__door_interrupt();
    init_tables();
    setTimestamp(0);
    
    uint8_t inital_door_state = initial_door_state();
    // write initial_door_state to table
    update_status(inital_door_state);

    // write to table
    int timestamp = getTimestamp();

    is_state_entry table_entry;
    table_entry.gateID = GATE_ID;
    table_entry.state = inital_door_state;
    table_entry.gateTime = timestamp;
    setTimestamp(timestamp++);

    // if (TABLE_SUCCESS == set_is_state_entry()){
    if (0 == set_is_state_entry(&table_entry)){
        

        // TODO REMOVE LATER #1
        // ---------------------------------------------
        if (!inital_door_state) {
            
            puts("door closed initially");
        }
        else {
            
            puts("door opened initially");
        }
    // ----------------------------------------------
    } else {
        //err = -1;
    }


    // TEST TABLE IS WRITTEN
    is_state_entry test_table_entry;
    if (!get_is_state_entry(GATE_ID, &test_table_entry)) {
        printf("TEST ENTRY Time %d\n", test_table_entry.gateTime);
        printf("TEST ENTRY State %d\n", test_table_entry.state);
    } else {
        printf("test failed");
    }

    // start lorawan

    if (!start_lorawan()){
        printf("starting lorawan failed");
        return -1;
    }

    // start thread init bluetooth
    // thread_create(
    //     ble_stack,
    //     sizeof(lorawan_stack),
    //     THREAD_PRIORITY_MAIN - 1,
    //     THREAD_CREATE_STACKTEST,
    //     ble_init,
    //     NULL,
    //    "ble"
    // };
    while(1){
        // TODO  error detection
        // err = check_door_status();

    }
    return 0;
}