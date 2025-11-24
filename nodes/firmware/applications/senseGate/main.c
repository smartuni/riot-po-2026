#include <stdio.h>

#include "od.h"
#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "tables.h"
#include "tables/records.h"
#include "mate_lorawan.h"
#include "mate_ble.h"
#include "event/thread.h"
#include "inductive_sensor.h"
#include "include/gate_observer.h"
#include "mtd.h"

#define STORAGE_RAM_MOUNT_PATH "/ram0"
#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH
extern int credential_manager_setup(const char *db_path);
extern int tables_setup(tables_context_t **t, const char *db_path);
extern int storage_setup_ram_mtd(const char *mount_path);
extern mtd_dev_t *storage_setup_get_ram_mtd(void);
tables_context_t *tables;

#define TIME_PERIOD_TABLE_UPDATE 30 // const defines time to update table periodically

/* PIN label on feather sense: "A1" */
#define REED_0_PIN_0 GPIO_PIN(0, 5)

#define INDUCTIVE_SENSOR_DCDC_PWR_PIN GPIO_PIN(0,4)
#define INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH (false)
#define INDUCTIVE_SENSOR_ADC_LINE (4)
#define INDUCTIVE_SENSOR_ADC_VREF_MV (3300)
#define INDUCTIVE_SENSOR_VREF_MV     (11000)

char ble_send_stack[2*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[2*THREAD_STACKSIZE_DEFAULT];

inductive_sensor_t inductive_sensor;

uint32_t inductive_sensor_measure_cb(void *ctx)
{
    inductive_sensor_t *is = (inductive_sensor_t*)ctx;
    inductive_sensor_power(is, true);
    ztimer_sleep(ZTIMER_MSEC, 50);
    uint32_t sum = 0;
    unsigned nsamples = 16;
    for (unsigned i = 0; i < nsamples; i++) {
       sum += inductive_sensor_sample(&inductive_sensor);
    }
    inductive_sensor_power(is, false);

    uint32_t avg = sum / nsamples;
    return inductive_sensor_sample2adc_voltage(is, avg);
}

gate_observer_t observer = {
    .config = {
        .distance_sensor_confs = {
                                   { .closed_min = 30,
                                     .closed_max = 2000,
                                     .measure_cb_ctx = &inductive_sensor,
                                     .measure_distance_cb = inductive_sensor_measure_cb },
                                 },
        .limit_switch_confs = {
                                { .pin = REED_0_PIN_0,
                                  .pull_conf = GPIO_IN_PU,
                                  .closed_level = false }
                              },
    },
};

void gate_observer_state_change_cb(gate_state_t new_state)
{
    int res = tables_put_gate_report(tables, new_state);
    printf("gate_observer_state_change_cb: %d\n", res);
}

static void _table_update_cb(tables_context_t *ctx, const table_record_t *record,
                             const table_query_t *query, void *arg)
{
    (void)ctx;
    (void)arg;
    (void)query;
    printf("\n_table_update_cb\n");
    table_record_type_t type;
    get_record_type(record, &type);
    printf("record type: %d\n", type);

    const node_id_t *writer_id;
    get_record_writer_id(record, &writer_id);
    od_hex_dump(writer_id, NODE_ID_SIZE, 0);

    record_sequence_t sequence;
    get_record_sequence(record, &sequence);
    printf("record sequence: %llu\n", sequence);
    printf("record sequence lower32: %lu\n", (uint32_t)sequence);

    if (type == RECORD_GATE_REPORT) {
        table_gate_report_t *report;
        int res = get_gate_report_data(record, &report);
        if (res) {
            printf("get_gate_report_data failed\n");
        } else {
            gate_state_t state;
            get_gate_report_state(report, &state);
            printf("gate state: %d\n", state);
        }
    }
}

int main(void){
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("[main]: starting");

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

    table_memo_t memo;
    table_query_t query;

    void *cb_arg = 0;
    tables_init_query(&query, RECORD_GATE_REPORT, NULL, NULL);
    tables_add_memo(tables, &memo, &query, _table_update_cb, cb_arg);

    int isi_res = inductive_sensor_init(&inductive_sensor,
                                        INDUCTIVE_SENSOR_DCDC_PWR_PIN,
                                        INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH,
                                        INDUCTIVE_SENSOR_ADC_LINE,
                                        INDUCTIVE_SENSOR_ADC_VREF_MV,
                                        INDUCTIVE_SENSOR_VREF_MV);

    if (isi_res != ANALOG_GATE_SENSOR_SUCCESS) {
        printf("[main]: inductive sensor init failed! %d\n", isi_res);
    }

    int goi_res = gate_observer_init(&observer, &observer.config,
                                     gate_observer_state_change_cb);

    if (goi_res != 0) {
        printf("[main]: gate_observer_init failed! %d\n", goi_res);
    }

    puts("[main]: reading initial door state");
    gate_observer_state_t obs_state;
    gate_state_t initial_gate_state = gate_observer_get_state(&observer, &obs_state);

    for (int i = 0; i < GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT; i++) {
        printf("[main]: limit switch[%d]: %s\n", i, obs_state.ls_states[i] == LIMIT_SWITCH_ENGAGED ? "CLOSED" : "OPEN");
    }

    for (int i = 0; i < GATE_OBSERVER_DISTANCE_SENSOR_CNT; i++) {
        uint32_t distance = obs_state.distances[i];
        printf("[gate_sensor]: distance[%d]: %lu\n", i, distance);
    }

    printf("[main]: initial gate state: %s\n",
            initial_gate_state == GATE_STATE_CLOSED ? "CLOSED" :
            (initial_gate_state == GATE_STATE_OPEN ? "OPEN" : "INVALID"));
    
    puts("[main]: put initial state to tables...");
    res = tables_put_gate_report(tables, initial_gate_state);
    printf("tables_put_gate_report: %d\n", res);

    // start lorawan
    puts("[main]: starting lorawan");
    int lorawanstarted = mate_lorawan_start(tables);
    if (-1 == lorawanstarted){
        printf("[main]: starting lorawan failed");
    }

    //start thread init bluetooth
    puts("[main]: starting ble");
    if (BLE_SUCCESS == ble_init()){
        puts("[main]: BLE init complete");
    } else {
        puts("[main]: BLE not started");
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
            lorawanstarted = mate_lorawan_start(tables);
        }
        
        increment_device_timestamp();
        ztimer_sleep(ZTIMER_MSEC,1000);
    
        if (timeToUpdateTable == TIME_PERIOD_TABLE_UPDATE) {
            printf("updating own reported state\n");
            gate_state_t current_gate_state = gate_observer_get_state(&observer, &obs_state);
            res = tables_put_gate_report(tables, current_gate_state);
            if (res == 0) {
                printf("tables_put_gate_report: %d\n", res);
            }
            timeToUpdateTable = 0;
        } else {
            timeToUpdateTable++;
        }

        

    }
    return 0;
}
