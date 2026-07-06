#include "od.h"
#include "ztimer.h"
#include <stdio.h>
#include <list.h>
#include "board.h"
#include "periph/gpio.h"
#include "ztimer.h"
#include "include/events_creation.h"
#include "tables.h"
#include "tables/records.h"
#include "mate_lorawan.h"
#include "include/sound.h"
#include "include/vibrationModule.h"
#include "include/sensemate_ui.h"
#include "mate_ble.h"
#include "mtd.h"
#include "shell.h"
#define LOG_LEVEL   LOG_INFO
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[main]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[main]: " __VA_ARGS__)
#include "ps.h"

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

#define STORAGE_RAM_MOUNT_PATH "/ram0"
//#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH

#if IS_USED(MODULE_FLASHDB_MTD)
#define STORAGE_MOUNT_PATH ""
#else
#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH
#endif

static int min_rssi = -100;

extern int credential_manager_setup(const char *db_path);
extern int tables_setup(tables_context_t **t, const char *db_path);
extern int storage_setup_ram_mtd(const char *mount_path);
extern mtd_dev_t *storage_setup_get_ram_mtd(void);
tables_context_t *tables;

#define SOUND_PWM_DEV PWM_DEV(0)
#if defined BOARD_ADAFRUIT_FEATHER_NRF52840_SENSE // v1 board
#define SOUND_PWM_CHANNEL (1)
#elif defined BOARD_SEEEDSTUDIO_XIAO_NRF52840_SENSE // v2 board
#define SOUND_PWM_CHANNEL (0)
#else
#error "Missing board-specific configuration."
#endif

sound_module_t sound_module;

int lorawan_started = -1;

table_iterator_t *_all_gates_iterator;
table_query_t all_gates_query = {
    .type = RECORD_GATE_REPORT,
    .writer_id = NULL,
    .involved_id = NULL
};

bool _set_min_rssi_cb(int8_t rssi)
{
    LOG_INFO("New minimum RSSI: %d\n", rssi);

    min_rssi = rssi;

    return true;
}

int8_t _get_min_rssi_cb(void){
    return min_rssi;
}

uint32_t _get_known_gate_count_by_type(table_record_type_t type)
{
    uint32_t gates_cnt = 0;
    TABLE_ITERATOR(iter, tables);
    table_query_t query;
    tables_init_query(&query, type, NULL, NULL);

    int res = tables_iterator_init(tables, &iter, &query);
    _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        return false;
    }

    table_record_t *record;

    while( tables_iterator_next(tables, &iter, &record, NULL, NULL) == 0) {
        _LOGDBG("%s iter next (%d) %s\n", __func__, res, ok(res == 0));
        gates_cnt++;
    }

    return gates_cnt;
}

uint32_t _get_known_gate_count(void)
{
    return _get_known_gate_count_by_type(RECORD_GATE_REPORT);
}

uint32_t _get_visible_mate_count(rssi_t min_rssi)
{
    uint32_t mate_cnt = 0;
    TABLE_ITERATOR(iter, tables);
    table_query_t query;
    tables_init_query(&query, RECORD_MATE_ENCOUNTER, NULL, NULL);

    int res = tables_iterator_init(tables, &iter, &query);
    _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        return false;
    }

    table_record_t *record;

    while( tables_iterator_next(tables, &iter, &record, NULL, NULL) == 0) {
        _LOGDBG("%s iter next (%d) %s\n", __func__, res, ok(res == 0));
        table_mate_encounter_t* data;
        rssi_t rssi;
        if(get_mate_encounter_data(record, &data) == 0){
            get_mate_encounter_rssi(data, &rssi);
            if(rssi > min_rssi){
                mate_cnt++;
            } else {
                _LOGDBG("%s rssi of mate is too weak, not considered visible\n", __func__);
            }
        }
   
    }

    return mate_cnt;
}

static bool _all_gates_iter(ui_data_element_t *prev)
{
    /* abort if the iterator was not setup yet */
    if (_all_gates_iterator == NULL) {
        return false;
    }

    if (prev->iter_ctx.ptr == NULL) {
        int res = tables_iterator_init(tables, _all_gates_iterator, &all_gates_query);
        _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
        if (res) {
            return false;
        }
    }

    table_record_t *record;

    int res = tables_iterator_next(tables, _all_gates_iterator, &record, NULL, NULL);
    _LOGDBG("%s iter next (%d) %s\n", __func__, res, ok(res == 0));
    if (res != 0) {
        prev->iter_ctx.ptr = NULL;
        return false;
    }

    gate_local_info_entry_t *li = &prev->data.local_gate_info;

    table_record_type_t type;
    get_record_type(record, &type);
    if (type == RECORD_GATE_REPORT) {
        table_gate_report_t *rdata;
        const node_id_t *writer_id;
        get_record_writer_id(record, &writer_id);
        _LOGDBG("writer id:\n");
        if (LOG_LEVEL >= LOG_DEBUG) {
            od_hex_dump(writer_id, sizeof(node_id_t), 0);
        }
        if (get_gate_report_data(record, &rdata) == 0) {
            _LOGDBG("%s Gate State: %s\n", __func__, gate_state_tostr(rdata->state));
            hlc_timestamp_t timestamp;
            get_record_timestamp(record, &timestamp);
            //li->sensor_timestamp = timestamp;
            memcpy(li->gateID, writer_id, sizeof(node_id_t));
            li->sensor_state = rdata->state;
            li->sensor_data_present = true;
            prev->iter_ctx.ptr = _all_gates_iterator;
            return true;
        }
    }
    return false;
}

static bool _put_gate_observation_cb(ui_data_element_t *elem)
{
    /* abort if the iterator was not setup yet */
    if (_all_gates_iterator == NULL) {
        return false;
    }

    table_gate_observation_t *obs = &elem->data.gate_observation;
    int res = tables_put_gate_observation(tables, &obs->gate_id, obs->state);
    _LOGINF("%s: %d %s %s\n", __func__,
                              obs->gate_id[3],
                              gate_state_tostr(obs->state),
                              ok(res == 0));
    if (res == 0) {
        /* changed */
        return true;
    } else {
        /* not changed */
        _LOGINF("%s put observation failed!.\n", __func__);
        return false;
    }
}

static ui_data_cbs_t _ui_data_cbs = {
    .all_gates_iter = _all_gates_iter,
    .put_gate_observation = _put_gate_observation_cb,
    .set_min_rssi = _set_min_rssi_cb,
    .get_min_rssi = _get_min_rssi_cb,
    .jobs_iter = NULL,
};

char shell_stack[2*THREAD_STACKSIZE_DEFAULT];

void* shell_thread(void* arg)
{
    (void)arg;
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

int main(void) {
    printf("init menu...\n");
    sensemate_ui_init(&_ui_data_cbs);
    ui_data_t *ui_state = sensemate_ui_get_state();
    ui_state->ble_state = ESTABLISHING_CONNECTION;
    sensemate_ui_update();

    //ztimer_sleep(ZTIMER_MSEC, 3000);
    int res = storage_setup_ram_mtd(STORAGE_MOUNT_PATH);
    _LOGDBG("storage_setup_ram_mtd: %s\n", ok(res == 0));

    res = credential_manager_setup(STORAGE_MOUNT_PATH "/cred");
    _LOGDBG("credential_manager_setup: %s\n", ok(res == 0));

    res = tables_setup(&tables, STORAGE_MOUNT_PATH "/tables");
    _LOGDBG("tables_setup: %s\n", ok(res == 0));

    TABLE_ITERATOR(all_gates_iterator, tables);
    _all_gates_iterator = &all_gates_iterator;

    res = sound_init(&sound_module, SOUND_PWM_DEV, SOUND_PWM_CHANNEL);
    _LOGDBG("sound_init: %s\n", ok(res == 0));
    init_vibration_module();
    //event_post(&sound_queue, &start_sound_event);

    printf("Device Type: %d device id: %d\n", RIOT_CONFIG_DEVICE_TYPE,
                                              RIOT_CONFIG_DEVICE_ID);
    init_event();

    thread_create(
        shell_stack,
        sizeof(shell_stack),
        THREAD_PRIORITY_MAIN - 2,
        THREAD_CREATE_STACKTEST,
        shell_thread,
        NULL,
       "shell"
    );

    //for (unsigned i = 0; i < 2; i++) {
    //    printf("put report %d\n", i);
    //    res = tables_put_gate_report(tables, GATE_STATE_OPEN);
    //}

    puts("starting ble");
    kernel_pid_t ble_tx_pid = KERNEL_PID_UNDEF;
    if (BLE_SUCCESS == mate_ble_init(tables, &ble_tx_pid)){
        puts("Ble init complete");
    } else {
        puts("BLE not started");
    }

    ui_state->lora_state = ESTABLISHING_CONNECTION;
    ui_state->ble_state = CONNECTED;
    sensemate_ui_update();

    lorawan_started = mate_lorawan_start(tables);

    puts("entering main loop");
    uint32_t prev_gate_cnt = 0;
    uint32_t prev_mate_cnt = 0;
    uint32_t ps_cnt = 0;
    bool join_done = false;
    while (1)
    {
        ui_state->visible_gate_cnt = _get_known_gate_count();
        ui_state->visible_mate_cnt = _get_visible_mate_count(min_rssi);
        ui_state->pending_jobs_cnt = _get_known_gate_count_by_type(RECORD_GATE_JOB);

        bool updateui = false;

        if (prev_gate_cnt != ui_state->visible_gate_cnt ||
            prev_mate_cnt != ui_state->visible_mate_cnt) {
            prev_gate_cnt = ui_state->visible_gate_cnt;
            prev_mate_cnt = ui_state->visible_mate_cnt;
            updateui = true;
        }

        if(!join_done && mate_lorawan_joined()) {
            ui_state->lora_state = CONNECTED;
            join_done = true;
            updateui = true;
        }

        if (updateui) {
            sensemate_ui_update();
        }
        ztimer_sleep(ZTIMER_MSEC, 1000);
        //if (ps_cnt >= 10) {
        //    printf("\n==============");
        //    ps();
        //    ps_cnt = 0;
        //}
        ps_cnt++;
    }

    printf("Display demo finished.\n");

    return 0;
}
