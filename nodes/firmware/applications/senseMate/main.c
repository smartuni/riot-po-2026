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
#include "mtd.h"

#define STORAGE_RAM_MOUNT_PATH "/ram0"
#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH
extern int credential_manager_setup(const char *db_path);
extern int tables_setup(tables_context_t **t, const char *db_path);
extern int storage_setup_ram_mtd(const char *mount_path);
extern mtd_dev_t *storage_setup_get_ram_mtd(void);
tables_context_t *tables;

char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

int lorawan_started = -1;

static bool _all_gates_iter(ui_data_element_t *prev)
{
    gate_id_t next_id = prev->iter_ctx.idx;
    gate_local_info_entry_t *li = &prev->data.local_gate_info;

    for (gate_id_t i = next_id; i < MAX_GATE_COUNT; i++) {
        if (tables_get_local_info_entry(i, li) == TABLE_SUCCESS) {
            /* remember where to continue on the next call */
            prev->iter_ctx.idx = i + 1;
            return true;
        }
    }
    return false;
}

static bool _set_gate_seen_state_cb(ui_data_element_t *elem)
{
    gate_seen_state_entry_t *seen_state = &elem->data.seen_state;

    printf("_set_gate_seen_state_cb: %d %d\n", seen_state->gateID, seen_state->state);
    /* populate remaining data which is not provided from the UI */
    seen_state->senseMateID = RIOT_CONFIG_DEVICE_ID;
    seen_state->timestamp = RIOT_CONFIG_DEVICE_ID;

    gate_timestamp_entry_t te;
    /* FIXME: if the gate was not seen before (and therefore there is no entry in the timestamp table)
     *        no seen state entry will be generated.
     *        In the future at least a local time reference should be added. */
    int res = get_timestamp_entry(seen_state->gateID, &te);
    if (res == TABLE_SUCCESS) {
        seen_state->timestamp = te.timestamp;
        int sres = set_seen_status_entry(seen_state);
        if (sres == TABLE_NEW_RECORD || sres == TABLE_UPDATED) {
            ////TODO: move to new tables API
            //event_post(EVENT_PRIO_MEDIUM, &send_seen_status_table);
            return true;
        }
    }
    return false;
}

static ui_data_cbs_t _ui_data_cbs = {
    .all_gates_iter = _all_gates_iter,
    .set_seen_state = _set_gate_seen_state_cb,
    .jobs_iter = NULL,
};

int main(void) {
    //ztimer_sleep(ZTIMER_MSEC, 3000);

    int res = storage_setup_ram_mtd(STORAGE_MOUNT_PATH);
    printf("storage_setup_ram_mtd: %d\n", res);
    mtd_dev_t *mtd = storage_setup_get_ram_mtd();
    printf("MTD '%s' device properties:\n", STORAGE_MOUNT_PATH);
    printf("sector_count:     %"PRIu32"   \n", mtd->sector_count);
    printf("pages_per_sector: %"PRIu32"   \n", mtd->pages_per_sector);
    printf("page_size:        %"PRIu32"   \n", mtd->page_size);

    res = credential_manager_setup(STORAGE_MOUNT_PATH "/cred");
    printf("credential_manager_setup: %d\n", res);

    res = tables_setup(&tables, STORAGE_MOUNT_PATH "/tables");
    printf("tables_setup: %d\n", res);


    printf("init menu...\n");
    sensemate_ui_init(&_ui_data_cbs);
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

    lorawan_started = mate_lorawan_start(tables);

    if (lorawan_started == 0) {
        ui_state->lora_state = CONNECTED;
        sensemate_ui_update();
    }

    puts("entering main loop");
    while (1)
    {
        if(lorawan_started == -1){
            lorawan_started = mate_lorawan_start(tables);
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
    test_seen.state = 1;
    test_seen.timestamp = 2000;
    test_seen.senseMateID = 3;
    int test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 2;
    test_seen.state = 0;
    test_seen.timestamp = 25000;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 3;
    test_seen.state = 1;
    test_seen.timestamp = 30000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 4;
    test_seen.state = 0;
    test_seen.timestamp = 28647;
    test_seen.senseMateID = 5;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 5;
    test_seen.state = 0;
    test_seen.timestamp = 1000;
    test_seen.senseMateID = 3;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 6;
    test_seen.state = 1;
    test_seen.timestamp = 5000;
    test_seen.senseMateID = 7;
    test_seen_merge = set_seen_status_entry(&test_seen);
    test_seen.gateID = 7;
    test_seen.state = 1;
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
