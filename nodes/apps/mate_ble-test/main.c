#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ztimer.h"
#include "mate_ble.h"

int main(void) 
{
    int status = ble_init();
    if (status != 0) {
        printf("ble_init failed with status 0x%X\n", status);
        return -1;
    }

    struct {
        uint8_t* buffer;
        int cbor_size;
        uint8_t* package_size;
        int capacity;
    } test_cbor_buffer_overwrite;

    char* testString = "Hi, this is a test string";

    test_cbor_buffer_overwrite.buffer = (uint8_t*)testString;
    test_cbor_buffer_overwrite.cbor_size = strlen(testString);
    test_cbor_buffer_overwrite.capacity = strlen(testString);

    printf("Sending test message\n");
    status = ble_send((cbor_buffer*)&test_cbor_buffer_overwrite);
    if (status != 0) {
        printf("ble_send failed with status 0x%X\n", status);
        return -1;
    }

    ble_metadata_t received_metadata = {};
    while (true) {
        ble_receive(187, (cbor_buffer*)&test_cbor_buffer_overwrite, &received_metadata);
        printf("Received message of type %d with rssi %d: %.*s\n", received_metadata.message_type, received_metadata.rssi, test_cbor_buffer_overwrite.cbor_size, test_cbor_buffer_overwrite.buffer);

        int strength = (float)(received_metadata.rssi + 100) / 100.f * 25.f;
        char buffer[28] = {};
        buffer[0] = '{';
        for (int i = 0; i < 25; i++)
        {
            buffer[i+1] = i + 1 <= strength ? '=' : ' ';
        }
        buffer[26] = '}';

        printf("Signal strength: %s\n", buffer);
    }

    return 0;
}