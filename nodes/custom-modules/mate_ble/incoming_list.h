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

#include "tables.h"
#include "mate_ble.h"

#include "architecture.h"

/**
 * @brief       Insert a new message into the incoming message list
 * @param[in]   data        Pointer to the data to insert
 * @param[in]   data_len    Length of the data to insert
 * @param[in]   metadata    Metadata associated with the message
 * @return      0 on success, negative error code on failure
 */
int insert_message(uint8_t* data, int data_len, ble_metadata_t metadata);

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

#include <string.h>
#include <mutex.h>
#include <stdio.h>

#include "ztimer.h"

#define MATE_BLE_INCOMING_LIST_SIZE (4)

typedef struct {
    cbor_buffer cbor_packet;
    ble_metadata_t metadata;
    uint8_t data[BLE_MAX_PAYLOAD_SIZE]; // Buffer to store the actual data
} incoming_message_t;

static incoming_message_t incoming_messages[MATE_BLE_INCOMING_LIST_SIZE];
static mutex_t list_mutex = MUTEX_INIT;

int insert_message(uint8_t* data, int data_len, ble_metadata_t metadata) 
{
    // Lock the mutex before accessing the shared data structure
    mutex_lock(&list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        if (incoming_messages[i].cbor_packet.buffer == NULL) {
            incoming_messages[i].cbor_packet.buffer = incoming_messages[i].data;
            incoming_messages[i].cbor_packet.cbor_size = data_len;
            incoming_messages[i].cbor_packet.capacity = BLE_MAX_PAYLOAD_SIZE;
            memcpy(incoming_messages[i].cbor_packet.buffer, data, data_len);

            printf("copied buffer to internal buffer %.*s\n", data_len, incoming_messages[i].cbor_packet.buffer);

            incoming_messages[i].metadata = metadata;
            mutex_unlock(&list_mutex);
            return BLE_SUCCESS;
        }
    }
    mutex_unlock(&list_mutex);
    return BLE_ERROR_INTERNAL_MESSAGE_BUFFER_FULL;
}

int remove_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata) 
{
    // Lock the mutex before accessing the shared data structure
    mutex_lock(&list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        if (incoming_messages[i].metadata.message_type == message_type && 
            incoming_messages[i].cbor_packet.buffer != NULL) {
            cbor_packet->cbor_size = incoming_messages[i].cbor_packet.cbor_size;
            cbor_packet->capacity = incoming_messages[i].cbor_packet.capacity;
            memcpy(cbor_packet->buffer, incoming_messages[i].cbor_packet.buffer, incoming_messages[i].cbor_packet.cbor_size);

            printf("copied buffer from internal buffer %.*s\n", cbor_packet->cbor_size, cbor_packet->buffer);

            *metadata = incoming_messages[i].metadata;

            memset(&incoming_messages[i], 0, sizeof(incoming_messages[i]));
            mutex_unlock(&list_mutex);
            return BLE_SUCCESS;
        }
    }
    mutex_unlock(&list_mutex);
    return BLE_ERROR_INTERNAL_NO_MESSAGE_FOUND;
}

int wait_for_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata)
{
    int result;
    while ((result = remove_message(message_type, cbor_packet, metadata)) == BLE_ERROR_INTERNAL_NO_MESSAGE_FOUND) {
        ztimer_sleep(ZTIMER_MSEC, 100); // Sleep for a short time before trying again
    }

    return result;
}