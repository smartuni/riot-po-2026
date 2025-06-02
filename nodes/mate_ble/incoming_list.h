/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    
 * @ingroup     
 * @brief       The incoming_list module stores incoming BLE messages. These messages are consumed by ble_receive()
 *
 * @{
 *
 * @file
 *
 * @author
 */

#ifndef MATE_BLE_INCOMING_LIST_H
#define MATE_BLE_INCOMING_LIST_H

#inclide "../tables/tables.h"

int insert_message(uint8_t* data, size_t data_len, ble_metadata_t metadata);
int remove_message(cbor_buffer* cbor_packet, ble_metadata_t* metadata);

#endif /* MATE_BLE_INCOMING_LIST_H */
