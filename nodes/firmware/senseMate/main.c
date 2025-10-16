#include "ztimer.h"
#include <stdio.h>
#include <list.h>
#include "board.h"
#include "periph/gpio.h"
#include "ztimer.h"
#include "include/interrupts.h"
#include "include/events_creation.h"
#include "tables.h"
#include "mate_lorawan.h"
#include "include/soundModule.h"
#include "include/vibrationModule.h"
#include "include/sensemate_ui.h"

#include "mate_ble.h"

//void fill_tables_test(void); //for menu testing purposes

char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

int lorawan_started = -1;

int main(void) {
    ztimer_sleep(ZTIMER_MSEC, 3000);
    printf("init menu...\n");
    sensemate_ui_init();
    ui_data_t *ui_state = sensemate_ui_get_state();

    init_interrupt();
    init_sound_module();
    init_vibration_module();
    //event_post(&sound_queue, &start_sound_event);

    printf("Device Type: %d device id: %d\n", RIOT_CONFIG_DEVICE_TYPE,
                                              RIOT_CONFIG_DEVICE_ID);
    init_event();
    init_tables();
    
    puts("starting ble");
    if (BLE_SUCCESS == ble_init()){
        puts("Ble init complete");
    } else {
        puts("BLE not started");
    }

    ble_tx_thread_args_t ble_tx_args = {
        .event_queue = &events_creation_queue,
        .tx_event = &eventBleTx,
    };

    thread_create(
        ble_send_stack,
        sizeof(ble_send_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        ble_send_loop,
        &ble_tx_args,
       "bleSend"
    );

    ble_receive_thread_args_t args = {
        .receive_queue = &events_creation_queue,
        .receive_news_event = &eventBleNews,
        .receive_any_event = &eventBleRx,
    };

    thread_create(
        ble_reicv_stack,
        sizeof(ble_reicv_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        ble_receive_loop,
        &args,
       "bleRecv"
    );

    ui_state->lora_state = ESTABLISHING_CONNECTION;
    ui_state->ble_state = CONNECTED;
    sensemate_ui_update();

    lorawan_started = start_lorawan();

    if (lorawan_started == 0) {
        ui_state->lora_state = CONNECTED;
        sensemate_ui_update();
    }

    puts("entering main loop");
    while (1)
    {
        if(lorawan_started == -1){
            lorawan_started = start_lorawan();
            if (lorawan_started == 0) {
                ui_state->lora_state = CONNECTED;
                sensemate_ui_update();
            }
        }
        ztimer_sleep(ZTIMER_MSEC, 1000);
        //refresh_display();
        increment_device_timestamp();
    }
    
    printf("Display demo finished.\n");

    return 0;
}

void fill_tables_test(void){
    gate_sensor_state_entry_t test;
    test.gateID = 1;
    test.state = 1;
    test.timestamp = 20000;
    int test_merge = set_is_state_entry(&test);
    test.gateID = 2;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 3;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 4;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 5;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 6;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    test.gateID = 7;
    test.state = 1;
    test.timestamp = 20000;
    test_merge = set_is_state_entry(&test);
    printf("Test merge: %d\n", test_merge);

    gate_seen_state_entry_t test_seen;
    test_seen.gateID = 1;
    test_seen.status = 1;
    test_seen.timestamp = 2000;
    test_seen.senseMateID = 3;
    int test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 2;
    test_seen.status = 0;
    test_seen.timestamp = 25000;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 3;
    test_seen.status = 1;
    test_seen.timestamp = 30000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 4;
    test_seen.status = 0;
    test_seen.timestamp = 28647;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 5;
    test_seen.status = 0;
    test_seen.timestamp = 1000;
    test_seen.senseMateID = 3;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 6;
    test_seen.status = 1;
    test_seen.timestamp = 5000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 7;
    test_seen.status = 1;
    test_seen.timestamp = 10000;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    printf("Test merge: %d\n", test_seen_merge);
    
    gate_target_state_entry_t test_target;
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

    gate_job_entry_t test_job;
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

    gate_timestamp_entry_t test_timestamp;
    test_timestamp.gateID = 1;
    test_timestamp.timestamp = 55000;
    test_timestamp.rssi = -50;
    int test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 2;
    test_timestamp.timestamp = 60000;
    test_timestamp.rssi = -100;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 3;
    test_timestamp.timestamp = 66000;
    test_timestamp.rssi = 0;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 4;
    test_timestamp.timestamp = 70000;
    test_timestamp.rssi = 170;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 5;
    test_timestamp.timestamp = 8000;
    test_timestamp.rssi = -20;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 6;
    test_timestamp.timestamp = 9000;
    test_timestamp.rssi = -30;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    test_timestamp.gateID = 7;
    test_timestamp.timestamp = 10000;
    test_timestamp.rssi = 0;
    test_timestamp_merge = set_timestamp_entry(&test_timestamp);
    printf("Test merge: %d\n", test_timestamp_merge);
}
