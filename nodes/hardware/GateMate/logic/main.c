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

#define TIME_PERIOD_TABLE_UPDATE 30 // const defines time to update table periodically


char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

int main(void){

    puts("starting");
    init__door_interrupt();
    puts("init tables");
    init_tables();
    
    puts("reading initial door state");
    uint8_t inital_door_state = initial_door_state();
    // write initial_door_state to table
    update_status(inital_door_state);

    // write to table
    puts("write to table");
    is_state_entry table_entry;
    table_entry.gateID = GATE_ID;
    table_entry.state = inital_door_state;
    table_entry.gateTime = get_device_timestamp();

    // if (TABLE_SUCCESS == set_is_state_entry()){
     if (TABLE_UPDATED == set_is_state_entry(&table_entry)){
        
        if (!inital_door_state) {
            
            puts("door closed initially");
        }
        else {
            
            puts("door opened initially");
        }
    } else {
        puts("could not write to table");
    }

    // start lorawan
    puts("starting lorawan");
    int lorawanstarted = start_lorawan();
    if (-1 == lorawanstarted){
        printf("starting lorawan failed");
    }


        //start thread init bluetooth
    puts("starting ble");
    if (BLE_SUCCESS == ble_init()){
        puts("Ble init complete");
    } else {
        puts("BLE not started");
    }


    thread_create(
        ble_send_stack,
        sizeof(ble_send_stack),
        THREAD_PRIORITY_MAIN - 2,
        THREAD_CREATE_STACKTEST,
        ble_send_loop,
        NULL,
       "bleSend"
    );
    
     thread_create(
         ble_reicv_stack,
         sizeof(ble_reicv_stack),
         THREAD_PRIORITY_MAIN - 3,
         THREAD_CREATE_STACKTEST,
         ble_receive_loop,
         NULL,
        "bleRecv"
     );

    int timeToUpdateTable = 0; // var to update table periodically
    
    while(1){

        
        if (-1 == lorawanstarted){
            lorawanstarted = start_lorawan();
        }
        
        increment_device_timestamp();
        ztimer_sleep(ZTIMER_MSEC,1000);
    
        if (timeToUpdateTable == TIME_PERIOD_TABLE_UPDATE) {
            is_state_entry table_update_entry;
            table_update_entry.gateID = GATE_ID;
            table_update_entry.state = get_status();
            table_update_entry.gateTime = get_device_timestamp();

            if (TABLE_UPDATED == set_is_state_entry(&table_update_entry)){
                puts("Table updated with newest timestamp");
            }
            timeToUpdateTable = 0;
        } else {
            timeToUpdateTable++;
        }

        

    }
    return 0;
}