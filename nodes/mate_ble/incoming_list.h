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
 * @file        incoming_list.h
 *
 * @author      Nico
 */

#ifndef MATE_BLE_INCOMING_LIST_H
#define MATE_BLE_INCOMING_LIST_H

#include "../tables/tables.h"
#include "mate_ble.h"

/**
 * @brief       Insert a new message into the incoming message list
 * @param[in]   data        Pointer to the data to insert
 * @param[in]   data_len    Length of the data to insert
 * @param[in]   metadata    Metadata associated with the message
 * @return      0 on success, negative error code on failure
 */
int insert_message(uint8_t* data, size_t data_len, ble_metadata_t metadata);

/**
 * @brief       Remove a message from the incoming message list
 * @param[in]   messageType Type of the message to remove
 * @param[out]  cbor_packet Pointer to the output cbor packet
 * @param[out]  metadata    Pointer to the output metadata
 * @return      0 on success, negative error code on failure
 */
int remove_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata);

/**
 * @brief       Wait for a message of a specific type in the incoming message list and remove it
 * @param[in]   messageType Type of the message to wait for and remove
 * @param[out]  cbor_packet Pointer to the output cbor packet
 * @param[out]  metadata    Pointer to the output metadata
 * @return      0 on success, negative error code on failure
 */
int wait_for_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata);

#endif /* MATE_BLE_INCOMING_LIST_H */
