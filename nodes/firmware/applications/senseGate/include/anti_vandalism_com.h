#pragma once
#ifndef ANTI_VANDALISM_COM_H
#define ANTI_VANDALISM_COM_H

#include "mate_lorawan.h"
#include <stdlib.h>

void do_fun_things(void) {
	table_query_t* q = (table_query_t*)malloc(sizeof(table_query_t));
	
	// q->type = RECORD_BATTERY_MONITOR;
	// q->writer_id = &self_node_id;
	// q->involved_id = NULL;

	// table_record_t* record = (table_record_t*)malloc(sizeof(table_record_t));
	// record->header = (table_record_header_t){
	// 	.type = RECORD_BATTERY_MONITOR,
	// 	.writer = &self_node_id,
	// 	.sequence = 0,
	// 	.timestamp = {0}
	// };

	// record->data = (battery_info_payload_t){
	// 	.status = CHARGING,
	// 	.voltage_mv = 3700
	// };

	send_tables(q);
}

#endif