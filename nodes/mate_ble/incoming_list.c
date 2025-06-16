#include "incoming_list.h"

#define MATE_BLE_INCOMING_LIST_SIZE (4)

typedef struct {
    cbor_buffer cbor_packet;
    ble_metadata_t metadata;
    uint8_t data[BLE_MAX_PAYLOAD_SIZE]; // Buffer to store the actual data
} incoming_message_t;

static incoming_message_t incoming_messages[MATE_BLE_INCOMING_LIST_SIZE];
static mutex_t incoming_list_mutex = MUTEX_INIT;

int insert_message(uint8_t* data, size_t data_len, ble_metadata_t metadata) 
{
    // Lock the mutex before accessing the shared data structure
    mutex_lock(&incoming_list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        if (incoming_messages[i].cbor_packet == NULL) {
            incoming_messages[i].cbor_packet->buffer = incoming_messages[i].data;
            memcpy(incoming_messages[i].cbor_packet->buffer, data, data_len);
            // parse the cbor packet to initialize the cbor_packet structure
            //incoming_messages[i].cbor_packet->cbor_size = 
            incoming_messages[i].metadata = metadata;
            mutex_unlock(&incoming_list_mutex);
            return BLE_SUCCESS;
        }
    }
    mutex_unlock(&incoming_list_mutex);
    return BLE_ERROR_INTERNAL_MESSAGE_BUFFER_FULL;
}

int remove_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata) 
{
    // Lock the mutex before accessing the shared data structure
    mutex_lock(&incoming_list_mutex);
    for (int i = 0; i < MATE_BLE_INCOMING_LIST_SIZE; i++) {
        if (incoming_messages[i].cbor_packet != NULL && 
            incoming_messages[i].metadata.message_type == message_type) {
            *cbor_packet = incoming_messages[i].cbor_packet;
            *metadata = incoming_messages[i].metadata;
            incoming_messages[i].cbor_packet = NULL; // Mark as empty
            mutex_unlock(&incoming_list_mutex);
            return BLE_SUCCESS;
        }
    }
    mutex_unlock(&incoming_list_mutex);
    return BLE_ERROR_INTERNAL_NO_MESSAGE_FOUND;
}

int wait_for_message(cbor_message_type_t message_type, cbor_buffer* cbor_packet, ble_metadata_t* metadata)
{
    int result;
    while ((result = remove_message(message_type, cbor_packet, metadata)) == BLE_ERROR_INTERNAL_NO_MESSAGE_FOUND) {
        ztimer_sleep(ZTIMER_MSEC, 100); // Sleep for a short time before retrying
    }

    return result;
}