#include "incoming_list.h"
#include <string.h>
#include <mutex.h>
#include <stdio.h>

#include "ztimer.h"
#include "cbor.h"

#define MATE_BLE_INCOMING_LIST_SIZE (4)

typedef struct {
    cbor_buffer cbor_packet;
    ble_metadata_t metadata;
    uint8_t data[BLE_MAX_PAYLOAD_SIZE]; // Buffer to store the actual data
    uint8_t package_size; // Buffer to store the actual data
    
} incoming_message_t;

#define _INIT_CBOR_BUFFER(msg) \
{ \
    (msg).cbor_packet.buffer = (msg).data; \
    (msg).cbor_packet.package_size = (msg).package_size; \
    (msg).cbor_packet.cbor_size = 1; \
    (msg).cbor_packet.capacity = BLE_MAX_PAYLOAD_SIZE; \
}

static incoming_message_t incoming_messages[MATE_BLE_INCOMING_LIST_SIZE];
static mutex_t list_mutex = MUTEX_INIT;

int insert_message(uint8_t* data, int data_len, ble_metadata_t metadata) 
{
    if (data_len > BLE_MAX_PAYLOAD_SIZE) {
        return BLE_ERROR_INTERNAL_INVALID_DATA_LENGTH;
    }

    CborParser cobr_parser;
    CborValue cbor_iter;
    cbor_parser_init(data, data_len, 0, &cobr_parser, &cbor_iter);

    CborValue cbor_type;
    if(cbor_value_enter_container(&cbor_iter, &cbor_type) != CborNoError) {
        return BLE_ERROR_INTERNAL_CBOR_PARSING_ISSUE;
    }

    int table_type = CBOR_MESSAGE_TYPE_UNKNOWN;
    if(!cbor_value_is_integer(&cbor_type) || 
        cbor_value_get_int(&cbor_type, &table_type) != CborNoError) {
        return BLE_ERROR_INTERNAL_CBOR_PARSING_ISSUE;
    }

    if (table_type == CBOR_MESSAGE_TYPE_UNKNOWN) {
        return BLE_ERROR_INTERNAL_CBOR_PARSING_ISSUE;
    }
    metadata.message_type = table_type;

    // Lock the mutex before accessing the shared data structure
    mutex_lock(&list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        if (incoming_messages[i].cbor_packet.buffer == NULL) {
            _INIT_CBOR_BUFFER(incoming_messages[i]);
            memcpy(incoming_messages[i].cbor_packet.buffer, data, data_len);
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
    bool any_type = message_type == CBOR_MESSAGE_TYPE_WILDCARD;

    // Lock the mutex before accessing the message list
    mutex_lock(&list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        bool type_match = incoming_messages[i].metadata.message_type == message_type;
        if ((type_match || any_type) && 
            incoming_messages[i].cbor_packet.buffer != NULL) {
            cbor_packet->cbor_size = incoming_messages[i].cbor_packet.cbor_size;
            cbor_packet->package_size[0] = incoming_messages[i].cbor_packet.package_size[0];
            if (cbor_packet->capacity < incoming_messages[i].cbor_packet.capacity) {
                mutex_unlock(&list_mutex);
                return BLE_ERROR_INTERNAL_INSUFFICIENT_CAPACITY;
            }
            memcpy(cbor_packet->buffer, incoming_messages[i].cbor_packet.buffer, incoming_messages[i].cbor_packet.capacity);
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