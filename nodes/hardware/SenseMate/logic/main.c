#include "displayDemo.h"
#include "ztimer.h"
#include <stdio.h>
#include <list.h>
#include "board.h"
#include "periph/gpio.h"
#include "ztimer.h"
#include "interrupts.h"
#include "events_creation.h"
#include "tables.h"
#include "mate_lorawan.h"
#include "new_menu.h"
#include "soundModule.h"
#include "vibrationModule.h"

void fill_tables_test(void);


int main(void) {
    //ztimer_sleep(ZTIMER_MSEC, 3000);
    init_interrupt();
    init_sound_module();
    init_vibration_module();
    start_vibration();
    startup_sound();
    stop_vibration();

    //printf("Display demo started.\n");
    init_display();
    printf("Display initialized.\n");
    //display_demo();
    //init_menu();
    init_new_menu();
    //set_current_meustate(INIT);
    update_menu_display();
    init_event();
    init_tables();
    //is_state_entry receiveide_is_state;
    //int hallo = get_is_state_entry(1, &receiveide_is_state);
    //printf("Gate Id %d\n", receiveide_is_state.gateID);
    fill_tables_test();
    
    update_menu_from_tables();
    //(void) test_merge;
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

void fill_tables_test(void){
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
    printf("Test merge: %d\n", test_merge);

    seen_status_entry test_seen;
    test_seen.gateID = 1;
    test_seen.status = 1;
    test_seen.gateTime = 2000;
    test_seen.senseMateID = 3;
    int test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 2;
    test_seen.status = 0;
    test_seen.gateTime = 25000;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 3;
    test_seen.status = 1;
    test_seen.gateTime = 30000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 4;
    test_seen.status = 0;
    test_seen.gateTime = 28647;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 5;
    test_seen.status = 0;
    test_seen.gateTime = 1000;
    test_seen.senseMateID = 3;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 6;
    test_seen.status = 1;
    test_seen.gateTime = 5000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 7;
    test_seen.status = 1;
    test_seen.gateTime = 10000;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    printf("Test merge: %d\n", test_seen_merge);
    
    target_state_entry test_target;
    test_target.gateID = 1;
    test_target.state = 1;
    test_target.timestamp = 20000;
    int test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 2;
    test_target.state = 0;
    test_target.timestamp = 25000;
    test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 3;
    test_target.state = 1;
    test_target.timestamp = 30000;
    test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 4;
    test_target.state = 0;
    test_target.timestamp = 28647;
    test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 5;
    test_target.state = 0;
    test_target.timestamp = 1000;
    test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 6;
    test_target.state = 1;
    test_target.timestamp = 5000;
    test_target_merge = set_target_state_entry(&test_target);
    test_target.gateID = 7;
    test_target.state = 1;
    test_target.timestamp = 10000;
    test_target_merge = set_target_state_entry(&test_target);
    printf("Test merge: %d\n", test_target_merge);

    jobs_entry test_job;
    test_job.gateID = 1;
    test_job.done = 1;
    test_job.priority = 1;
    int test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 2;
    test_job.done = 0;
    test_job.priority = 2;
    test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 3;
    test_job.done = 0;
    test_job.priority = 3;
    test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 4;
    test_job.done = 0;
    test_job.priority = 2;
    test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 5;
    test_job.done = 0;
    test_job.priority = 0;
    test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 6;
    test_job.done = 0;
    test_job.priority = 3;
    test_job_merge = set_jobs_entry(&test_job);
    test_job.gateID = 7;
    test_job.done = 0;
    test_job.priority = 2;
    test_job_merge = set_jobs_entry(&test_job);
    printf("Test merge: %d\n", test_job_merge);

    timestamp_entry test_timestamp;
    test_timestamp.gateID = 1;
    test_timestamp.timestamp = 55000;
    test_timestamp.rssi = 50;
    int test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 2;
    test_timestamp.timestamp = 60000;
    test_timestamp.rssi = 60;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 3;
    test_timestamp.timestamp = 66000;
    test_timestamp.rssi = 60;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 4;
    test_timestamp.timestamp = 70000;
    test_timestamp.rssi = 170;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 5;
    test_timestamp.timestamp = 8000;
    test_timestamp.rssi = 80;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 6;
    test_timestamp.timestamp = 9000;
    test_timestamp.rssi = 70;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 7;
    test_timestamp.timestamp = 10000;
    test_timestamp.rssi = 90;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    printf("Test merge: %d\n", test_timestamp_merge);
}