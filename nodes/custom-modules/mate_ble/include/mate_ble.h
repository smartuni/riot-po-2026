/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    mate_ble
 * @ingroup     sys
 * @brief       BLE send and receive functionality for the sense mate / gate mate project
 *
 * @file
 *
 * @author     Nico
 */

#ifndef MATE_BLE_H
#define MATE_BLE_H

#include "event.h"
#include "tables.h"

#define BLE_SEND_INTERVAL (10000) // in milliseconds

#define BLE_SUCCESS (0)
#define BLE_ERROR (-1)
#define BLE_ERROR_INTERNAL_MESSAGE_BUFFER_FULL (-2)
#define BLE_ERROR_INTERNAL_NO_MESSAGE_FOUND (-3)
#define BLE_ERROR_INTERNAL_CBOR_PARSING_ISSUE (-4)
#define BLE_ERROR_INTERNAL_INSUFFICIENT_CAPACITY (-5)
#define BLE_ERROR_INTERNAL_INVALID_DATA_LENGTH (-6)

#define BLE_MAX_PAYLOAD_SIZE (300)

typedef uint8_t cbor_message_type_t;

#define CBOR_MESSAGE_TYPE_UNKNOWN ((uint8_t)-1)
#define CBOR_MESSAGE_TYPE_WILDCARD ((uint8_t)-2)

typedef struct {
   cbor_message_type_t message_type;
   int8_t rssi;
} ble_metadata_t;

typedef struct {
   event_queue_t* receive_queue;
   event_t* receive_event;
} ble_received_thread_args_t;


/**
 * @brief           Loop that manages the propagation of state tables via ble
 */
void ble_run_propagation(void);

/**
 * @brief           Blocking function to receive data over BLE
 * @param[in]       type    Type of data to receive
 * @param[inout]    cbor_packet  Pointer to output cbor packet
 * @param[inout]    metadata  Pointer to output metadata
 * @return          0 on success, negative error code on failure
 */
int ble_receive(cbor_message_type_t type, cbor_buffer* cbor_packet, ble_metadata_t* metadata);

/**
 * @brief           Function to send data over BLE
 * @param[in]       data    Pointer to the data to send
 * @return          0 on success, negative error code on failure
 */
int ble_send(cbor_buffer* cbor_packet);

/**
 * @brief           Function to initialize the BLE module
 * @return          0 on success, negative error code on failure
 */
int ble_init(void);

/**
 * @brief           Sender loop of the BLE module. Propagates the state tables
 *                  via BLE advertisements.
 */
void* ble_send_loop(void*);

/**
 * @brief           Receiver loop of the BLE module. Receives the propagated state tables
 */
void* ble_receive_loop(void* args);

#endif /* MATE_BLE_H */