#include "displayDemo.h"
#include "ztimer.h"
#include <stdio.h>
#include <list.h>
#include "board.h"
#include "periph/gpio.h"
#include "ztimer.h"
#include "interrupts.h"
#include "menu.h"
#include "events_creation.h"
#include "tables.h"
#include "mate_lorawan.h"




int main(void) {
    ztimer_sleep(ZTIMER_MSEC, 3000);
    //init_interrupt();

    //printf("Display demo started.\n");
    //init_display();
    //printf("Display initialized.\n");
    //display_demo();
    //init_menu();
    //set_current_meustate(INIT);
    //refresh_display();
    init_event();
    init_tables();
    is_state_entry test;
    test.gateID = 1;
    test.state = 1;
    test.gateTime = 20000;
    int test_merge = set_is_state_entry(&test);
    test.gateID = 2;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 3;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 4;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 5;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 6;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 7;
    test.state = 1;
    test.gateTime = 20000;
    test_merge = set_is_state_entry(&test);

    //is_state_entry receiveide_is_state;
    //int hallo = get_is_state_entry(1, &receiveide_is_state);
    //printf("Gate Id %d\n", receiveide_is_state.gateID);

    (void) test_merge;
    //(void) hallo;
    // const is_state_entry* table = get_is_state_table();
    // for (int i = 0; i < 255; i++){
    //     printf("ID: %d, State: %d, Time: %d\n", table[i].gateID, table[i].state, table[i].gateTime);
    // }

    start_lorawan();

    while (1)
    {
        ztimer_sleep(ZTIMER_MSEC, 200);
        //refresh_display();
    }
    
    printf("Display demo finished.\n");

    return 0;
}