#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "tables.h"
#include "mate_lorawan.h"
#include "mate_ble.h"

#include <stdio.h>

#include "header/detectDoorStatus.h"
#include "header/event_creation.h"


char lorawan_stack[THREAD_STACKSIZE_DEFAULT];
char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

int main(void){

    puts("starting");
    init__door_interrupt();
    puts("init tables");
    init_tables();
    setTimestamp(0);
    
    puts("reading initial door state");
    uint8_t inital_door_state = initial_door_state();
    // write initial_door_state to table
    update_status(inital_door_state);

    // write to table
    puts("write to table");
    int timestamp = getTimestamp();

    is_state_entry table_entry;
    table_entry.gateID = GATE_ID;
    table_entry.state = inital_door_state;
    table_entry.gateTime = timestamp++;
    setTimestamp(timestamp);

    // if (TABLE_SUCCESS == set_is_state_entry()){
     if (TABLE_UPDATED == set_is_state_entry(&table_entry)){
        

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
        puts("could not write to table");
       
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
    puts("starting lorawan");
    if (!start_lorawan()){
        printf("starting lorawan failed");
        return -1;
    }


        //start thread init bluetooth
    // puts("starting ble");
    // if (BLE_SUCCESS == ble_init()){
    //     puts("Ble init complete");
    // } else {
    //     puts("BLE not started");
    // }


    // thread_create(
    //     ble_send_stack,
    //     sizeof(ble_send_stack),
    //     THREAD_PRIORITY_MAIN - 1,
    //     THREAD_CREATE_STACKTEST,
    //     ble_send_loop,
    //     NULL,
    //    "bleSend"
    // );
    
    // thread_create(
    //     ble_reicv_stack,
    //     sizeof(ble_reicv_stack),
    //     THREAD_PRIORITY_MAIN - 1,
    //     THREAD_CREATE_STACKTEST,
    //     ble_receive_loop,
    //     NULL,
    //    "bleRecv"
    // );


    puts("main loop");
    while(1){
        
        // TODO  error detection
        // err = check_door_status();

    }
    return 0;
}