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
#define LOG_LEVEL   LOG_DEBUG
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[main]: " __VA_ARGS__)

#include "personalization.h"
#define MAX_SIGNATURE_SIZE 80
#define MAX_SERIALIZED_RECORD_SIZE 128

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

#define STORAGE_RAM_MOUNT_PATH "/ram0"
#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH
extern int credential_manager_setup(const char *db_path);
extern int tables_setup(tables_context_t **t, const char *db_path);
extern int storage_setup_ram_mtd(const char *mount_path);
tables_context_t *tables;

#define TIME_PERIOD_TABLE_UPDATE 30 // const defines time to update table periodically
//#define TIME_PERIOD_TABLE_UPDATE 2 // const defines time to update table periodically

/* PIN label on feather sense: "A1" */
#define REED_0_PIN_0 GPIO_PIN(0, 5)

#define INDUCTIVE_SENSOR_DCDC_PWR_PIN GPIO_PIN(0,4)
#define INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH (false)
#define INDUCTIVE_SENSOR_ADC_LINE (4)
#define INDUCTIVE_SENSOR_ADC_VREF_MV (3300)
#define INDUCTIVE_SENSOR_VREF_MV     (11000)

char ble_send_stack[8*THREAD_STACKSIZE_DEFAULT];
char ble_reicv_stack[8*THREAD_STACKSIZE_DEFAULT];

extern event_t *_mateble_send_event;
extern event_queue_t *_mateble_send_queue;

#include "shell.h"
char shell_stack[2*THREAD_STACKSIZE_DEFAULT];
void* shell_thread(void* arg)
{
    (void)arg;
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

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
    _LOGDBG("%s new state = %s\n", __func__, gate_state_tostr(new_state));
    int res = tables_put_gate_report(tables, new_state);
    _LOGDBG("%s tables_put_gate_report %s\n", __func__, ok(res == 0));
    // if ble-mate is ready to accept events
    if (_mateble_send_queue && _mateble_send_event) {
        // trigger ble TX
        event_post(_mateble_send_queue, _mateble_send_event);
    }
}

static void _table_update_cb(tables_context_t *ctx, const table_record_t *record,
                             const table_query_t *query, void *arg)
{
    (void)ctx;
    (void)arg;
    (void)query;
    table_record_type_t type;
    get_record_type(record, &type);
    _LOGDBG("%s record type: %s\n", __func__, record_type_tostr(type));
    if (LOG_LEVEL >= LOG_DEBUG) {
        const node_id_t *writer_id;
        get_record_writer_id(record, &writer_id);
        od_hex_dump(writer_id, NODE_ID_SIZE, 0);
    }
    if (type == RECORD_GATE_REPORT) {
        table_gate_report_t *report;
        int res = get_gate_report_data(record, &report);
        if (res) {
            _LOGDBG("get_gate_report_data failed\n");
        } else {
            gate_state_t state;
            get_gate_report_state(report, &state);
            _LOGDBG("gate state: %s\n", gate_state_tostr(state));
        }
    }

    // don't unconditionally trigger TX here.
    // The change may aswell come from a recent RX.
}


int main(void){
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("[main]: starting");

    thread_create(
        shell_stack,
        sizeof(shell_stack),
        THREAD_PRIORITY_MAIN - 2,
        THREAD_CREATE_STACKTEST,
        shell_thread,
        NULL,
       "shell"
    );

    int res = storage_setup_ram_mtd(STORAGE_MOUNT_PATH);
    _LOGDBG("storage_setup_ram_mtd: %s\n", ok(res == 0));

    res = credential_manager_setup(STORAGE_MOUNT_PATH "/cred");
    _LOGDBG("credential_manager_setup: %s\n", ok(res == 0));

    res = tables_setup(&tables, STORAGE_MOUNT_PATH "/tables");
    _LOGDBG("tables_setup: %s\n", ok(res == 0));

    table_memo_t memo;
    table_query_t query;

    void *cb_arg = 0;
    tables_init_query(&query, RECORD_GATE_REPORT, NULL, NULL);
    tables_add_memo(tables, &memo, &query, _table_update_cb, cb_arg);

    res = inductive_sensor_init(&inductive_sensor,
                                INDUCTIVE_SENSOR_DCDC_PWR_PIN,
                                INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH,
                                INDUCTIVE_SENSOR_ADC_LINE,
                                INDUCTIVE_SENSOR_ADC_VREF_MV,
                                INDUCTIVE_SENSOR_VREF_MV);
    _LOGDBG("inductive_sensor_init %s\n", ok(res == ANALOG_GATE_SENSOR_SUCCESS));

    res = gate_observer_init(&observer, &observer.config, gate_observer_state_change_cb);
    _LOGDBG("gate_observer_init %s\n", ok(res == 0));

    _LOGDBG("reading initial door state\n");
    gate_observer_state_t obs_state;
    gate_state_t initial_gate_state = gate_observer_get_state(&observer, &obs_state);

    for (int i = 0; i < GATE_OBSERVER_LIMITSWITCH_SENSOR_CNT; i++) {
        _LOGDBG("limit switch[%d]: %s\n", i, obs_state.ls_states[i] == LIMIT_SWITCH_ENGAGED ? "CLOSED" : "OPEN");
    }

    for (int i = 0; i < GATE_OBSERVER_DISTANCE_SENSOR_CNT; i++) {
        uint32_t distance = obs_state.distances[i];
        _LOGDBG("distance[%d]: %lu\n", i, distance);
    }

    _LOGDBG("initial gate state: %s\n",
            initial_gate_state == GATE_STATE_CLOSED ? "CLOSED" :
            (initial_gate_state == GATE_STATE_OPEN ? "OPEN" : "INVALID"));
    
    _LOGDBG("put initial state to tables...\n");
    res = tables_put_gate_report(tables, initial_gate_state);
    _LOGDBG("put initial gate report %s\n", ok(res == 0));

    // start lorawan
    _LOGDBG("starting lorawan...\n");
    int lorawanstarted = mate_lorawan_start(tables);
    if (-1 == lorawanstarted){
        _LOGDBG("starting lorawan failed\n");
    }

    //start thread init bluetooth
    _LOGDBG("starting [mate_ble]\n");
    if (BLE_SUCCESS == mate_ble_init(tables)){
        _LOGDBG("[mate_ble] init complete\n");
    } else {
        _LOGDBG("[mate_ble] not started\n");
    }

    thread_create(
        ble_send_stack,
        sizeof(ble_send_stack),
        THREAD_PRIORITY_MAIN - 3,
        //THREAD_PRIORITY_MAIN - 2,
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
    int put_cnt = 0;
    int put_err_cnt = 0;
    while(1){

        if (-1 == lorawanstarted){
            lorawanstarted = mate_lorawan_start(tables);
        }

        ztimer_sleep(ZTIMER_MSEC,1000);

        if (timeToUpdateTable == TIME_PERIOD_TABLE_UPDATE) {
            _LOGDBG("get current observer state...\n");
            gate_state_t current_gate_state = gate_observer_get_state(&observer, &obs_state);
            _LOGDBG("current state: %s\n", gate_state_tostr(current_gate_state));
            _LOGDBG("put new reported state...\n");
            res = tables_put_gate_report(tables, current_gate_state);
            _LOGDBG("tables_put_gate_report %s\n", ok(res == 0));
            put_cnt++;
            if (res) {
                put_err_cnt++;
            }
            timeToUpdateTable = 0;
        } else {
            timeToUpdateTable++;
        }
    }

    return 0;
}
