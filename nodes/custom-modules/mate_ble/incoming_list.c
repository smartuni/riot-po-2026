#include "incoming_list.h"

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