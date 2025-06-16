#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "table.h" // TODO FIX PATH
#include "lorawan.h" // TODO FIX PATH

#include <stdio.h>

#include "header/detectDoorStatus.h"


char lorawan_stack[THREAD_STACKSIZE_DEFAULT];
char ble_stack[THREAD_STACKSIZE_DEFAULT];

int main(void){
    init__door_interrupt();
    init_tables();
    
    int inital_door_state = initial_door_state();
    // TODO write initial_door_state to table
    set_status(uint8_t newStatus);

    // TODO REMOVE LATER #1
    // ---------------------------------------------
    if (!inital_door_state) {
        
        puts("door closed initially");
    }
    else {
        
        puts("door opened initially");
    }
    // ----------------------------------------------



    // TODO  start thread init lorawan
    thread_create(
        lorawan_stack,
        sizeof(lorawan_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        start_lorawan,
        NULL,
        "lorawan"
    );


    // start thread init bluetooth
    // thread_create(
    //     ble_stack,
    //     sizeof(lorawan_stack),
    //     THREAD_PRIORITY_MAIN - 1,
    //     THREAD_CREATE_STACKTEST,
    //     ble_init,
    //     NULL,
    //     "ble"
    // );
    
    while(1){
        // TODO  error detection
        // err = check_door_status();

    }
    return 0;
}