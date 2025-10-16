#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "tables.h"
#include "mate_lorawan.h"
#include "mate_ble.h"

#include <stdio.h>

#include "include/detectDoorStatus.h"
#include "include/event_creation.h"

#define TIME_PERIOD_TABLE_UPDATE 30 // const defines time to update table periodically


char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

int main(void){
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);

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
    table_entry.gateID = RIOT_CONFIG_DEVICE_ID;
    table_entry.state = inital_door_state;
    table_entry.timestamp = get_device_timestamp();

    int sis_res = set_is_state_entry(&table_entry);
    if (TABLE_NEW_RECORD == sis_res){
        if (!inital_door_state) {
            puts("door closed initially");
        }
        else {
            puts("door opened initially");
        }
    } else {
        puts("could not write initial gate state to table");
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
            table_update_entry.gateID = RIOT_CONFIG_DEVICE_ID;
            table_update_entry.state = get_status();
            table_update_entry.timestamp = get_device_timestamp();

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
