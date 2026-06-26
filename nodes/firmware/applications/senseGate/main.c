#include <stdio.h>
#include <stdlib.h>

#include "health_monitor.h"
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

#include "identity_store.h"
#define LOG_LEVEL LOG_DEBUG
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[main]: " __VA_ARGS__)

#include "personalization.h"
#define MAX_SIGNATURE_SIZE 80
#define MAX_SERIALIZED_RECORD_SIZE 128

#define STORAGE_RAM_MOUNT_PATH "/ram0"
//#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH

#if IS_USED(MODULE_FLASHDB_MTD)
#define STORAGE_MOUNT_PATH ""
#else
#define STORAGE_MOUNT_PATH STORAGE_RAM_MOUNT_PATH
#endif

extern int credential_manager_setup(const char* db_path);
extern int tables_setup(tables_context_t** t, const char* db_path);
extern int storage_setup_ram_mtd(const char* mount_path);
tables_context_t* tables;

#define TIME_PERIOD_TABLE_UPDATE 30 // const defines time to update table periodically
//#define TIME_PERIOD_TABLE_UPDATE 2 // const defines time to update table periodically

/* PIN label on feather sense: "A1" */
#define REED_0_PIN_0 GPIO_PIN(0, 5)

#define INDUCTIVE_SENSOR_DCDC_PWR_PIN GPIO_PIN(0, 4)
#define INDUCTIVE_SENSOR_DCDC_PWR_PIN_AH (false)
#define INDUCTIVE_SENSOR_ADC_LINE (4)
#define INDUCTIVE_SENSOR_ADC_VREF_MV (3300)
#define INDUCTIVE_SENSOR_VREF_MV (11000)

#include "shell.h"
char shell_stack[2 * THREAD_STACKSIZE_DEFAULT];
void* shell_thread(void* arg) {
	(void)arg;
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

	return 0;
}

int main(void) {
	ztimer_sleep(ZTIMER_MSEC, 7000);
	shock_detector_t* shock_detector = shock_detector_new(1000, 1024, 1);
	shock_detector_start(shock_detector);

	while (1) {
		ztimer_sleep(ZTIMER_MSEC, 1000);
		_LOGDBG("main loop\n");
		//shock_status_t shock_status;
		//shock_detector_fetch_status(shock_detector,&shock_status);
		//_LOGDBG("shock status: %d\n", (int)shock_status);
	}

	return 0;
}
