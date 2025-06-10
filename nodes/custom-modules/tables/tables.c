#include "tables.h"
#include <string.h>
#include "embUnit.h"
#include "embUnit/embUnit.h"
#include "fmt.h"
#include "cbor.h"


// Static tables
static target_state_entry target_state_entry_table[MAX_GATE_COUNT];
static is_state_entry is_state_entry_table[MAX_GATE_COUNT];
static seen_status_entry seen_status_entry_table[MAX_GATE_COUNT];
static jobs_entry jobs_entry_table[MAX_GATE_COUNT];

int target_state_table_to_cbor(cbor_buffer* buffer) {

    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_simple_value(&singleEntryEncoder, target_state_entry_table[i].gateID);
        cbor_encode_simple_value(&singleEntryEncoder, target_state_entry_table[i].state);
        cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].timestamp);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);
    buffer->capacity = 0;

    return 0;
}

int is_state_table_to_cbor(is_state_entry table[], cbor_buffer* buffer) {
    (void) table;
    (void) buffer;
    return 0;
}

int seen_status_table_to_cbor(seen_status_entry table[], cbor_buffer* buffer) {
    (void) table;
    (void) buffer;
    return 0;
}

int jobs_table_to_cbor(jobs_entry table[], cbor_buffer* buffer) {
    (void) table;
    (void) buffer;
    return 0;
}

int cbor_to_table(cbor_buffer* buffer) {
    (void) buffer;

    CborParser parser;
    CborValue it;

/**
    for (size_t idx = 0; idx < ARRAY_SIZE(tests); idx++) {
        CborParser parser;
        CborValue it;
        unsigned char buf[64] = {0};
        TEST_ASSERT((strlen(tests[idx])/2) <= sizeof(buf));

        size_t len = fmt_hex_bytes(buf, tests[idx]);
        TEST_ASSERT(len);

        CborError err = cbor_parser_init(buf, len, CborValidateStrictMode, &parser, &it);
        if(err != 0) {
            return -1;
        }

        return 0;
    }
**/
    return 0;
}

int target_state_table_to_cbor_many(target_state_entry table[], int package_size, cbor_buffer* buffer) {
    (void) package_size;
    (void) table;
    (void) buffer;
    return 0;
}