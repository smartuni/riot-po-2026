#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
//#include "tables.h" // TODO FIX PATH
#include "tables.h"
//#include "lorawan.h" // TODO FIX PATH
#include "mate_lorawan.h"

#include <stdio.h>

#include "header/detectDoorStatus.h"
#include "header/event_creation.h"


char lorawan_stack[THREAD_STACKSIZE_DEFAULT];
char ble_stack[THREAD_STACKSIZE_DEFAULT];

int main(void){
    init__door_interrupt();
    init_tables();
    
    int inital_door_state = initial_door_state();
    // TODO write initial_door_state to table
    update_status(inital_door_state);

    // TODO REMOVE LATER #1
    // ---------------------------------------------
    if (!inital_door_state) {
        
        puts("door closed initially");
    }
    else {
        
        puts("door opened initially");
    }
    // ----------------------------------------------



    // TODO  start lorawan

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