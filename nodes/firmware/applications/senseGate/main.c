#include <stdio.h>

#include "board.h"
#include "ztimer.h"
#include "thread.h"
#include "tables_old.h"
#include "tables.h"
#include "mate_lorawan.h"
#include "mate_ble.h"
#include "event/thread.h"
#include "inductive_sensor.h"
#include "include/gate_observer.h"

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
    /* update own state entry in table */
    gate_sensor_state_entry_t table_entry;
    table_entry.gateID = RIOT_CONFIG_DEVICE_ID;
    table_entry.state = new_state;
    table_entry.timestamp = get_device_timestamp();

    int tableUpdate = set_is_state_entry(&table_entry);
    if( TABLE_UPDATED == tableUpdate){
        /* inform lorawan to send an update */
        event_post(EVENT_PRIO_MEDIUM, &send_is_state_table);
    } else {
        puts("[main]: writing to table failed!");
    }
}

int main(void){
    int isi_res = inductive_sensor_init(&inductive_sensor,
                                        INDUCTIVE_SENSOR_DCDC_PWR_PIN,
                                        INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH,
                                        INDUCTIVE_SENSOR_ADC_LINE,
                                        INDUCTIVE_SENSOR_ADC_VREF_MV,
                                        INDUCTIVE_SENSOR_VREF_MV);

    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);

    puts("[main]: starting");

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

    puts("[main]: init tables");
    init_tables();
    
    // write to table
    puts("[main]: write to table");
    gate_sensor_state_entry_t table_entry;
    table_entry.gateID = RIOT_CONFIG_DEVICE_ID;
    table_entry.state = initial_gate_state;
    table_entry.timestamp = get_device_timestamp();

    int sis_res = set_is_state_entry(&table_entry);
    if (TABLE_NEW_RECORD == sis_res){
    } else {
        puts("[main]: could not write initial gate state to table");
    }

    // start lorawan
    puts("[main]: starting lorawan");
    int lorawanstarted = start_lorawan();
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
            lorawanstarted = start_lorawan();
        }
        
        increment_device_timestamp();
        ztimer_sleep(ZTIMER_MSEC,1000);
    
        if (timeToUpdateTable == TIME_PERIOD_TABLE_UPDATE) {
            gate_sensor_state_entry_t table_update_entry;
            table_update_entry.gateID = RIOT_CONFIG_DEVICE_ID;
            table_update_entry.state = gate_observer_get_state(&observer, NULL);
            table_update_entry.timestamp = get_device_timestamp();

            if (TABLE_UPDATED == set_is_state_entry(&table_update_entry)){
                puts("[main]: Table updated with newest timestamp");
            }
            timeToUpdateTable = 0;
        } else {
            timeToUpdateTable++;
        }

        

    }
    return 0;
}
