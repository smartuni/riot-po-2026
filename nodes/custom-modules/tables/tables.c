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

int target_state_table_to_cbor_test(target_state_entry table[], cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 2); // Entry 2

    // [Table Entry]
    for(int i = 0; i < 2; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_int(&singleEntryEncoder, table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, table[i].state);
        cbor_encode_int(&singleEntryEncoder, table[i].timestamp);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size = (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int target_state_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].state);
        cbor_encode_int(&singleEntryEncoder, target_state_entry_table[i].timestamp);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int is_state_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].state);
        cbor_encode_int(&singleEntryEncoder, is_state_entry_table[i].gateTime);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int seen_status_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].gateTime);
        cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].status);
        cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[i].senseMateID);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int jobs_table_to_cbor(cbor_buffer* buffer) {
    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buffer->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2

    // [Table Entry]
    for(int i = 0; i < MAX_GATE_COUNT; i++) {
        cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
        cbor_encode_int(&singleEntryEncoder, jobs_entry_table[i].gateID);
        cbor_encode_int(&singleEntryEncoder, jobs_entry_table[i].done);
        cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
    }

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buffer->cbor_size =  (uint8_t) cbor_encoder_get_buffer_size (&encoder, buffer->buffer);

    return 0;
}

int cbor_to_table_test(cbor_buffer* buffer) {
    CborParser parser;
    CborValue value;
    CborValue wrapperValue;
    CborValue fieldsValue;
    CborValue entryValue;

    int tableType;

    target_state_entry returnTargetTable[buffer->cbor_size];
    is_state_entry returnIsTable[buffer->cbor_size];
    seen_status_entry returnSeenTable[buffer->cbor_size];
    jobs_entry returnJobsTable[buffer->cbor_size];

    cbor_parser_init(buffer->buffer, buffer->cbor_size, 0, &parser, &value);
    if(cbor_value_enter_container(&value, &wrapperValue) != CborNoError) {
        return -1;
    }
    if(!cbor_value_is_integer(&wrapperValue) || cbor_value_get_int(&wrapperValue, &tableType) != CborNoError) {
        return -1;
    } // get type of table

    // [ enter second container
    cbor_value_advance(&wrapperValue);
    if(cbor_value_enter_container(&wrapperValue, &fieldsValue) != CborNoError) {
        return -1;
    }

    int id, s, sID, d, ts, gt;
    size_t length = 0;
    cbor_value_get_array_length(&fieldsValue, &length); 	
    for(size_t i = 0; i < (length - 1); i++) {
        cbor_value_enter_container(&fieldsValue, &entryValue); // [
        switch(tableType) {
            case TARGET_STATE_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &ts) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                target_state_entry newTargetEntry = {id, s, ts};
                returnTargetTable[i] = newTargetEntry;
                break;
            case IS_STATE_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &gt) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                is_state_entry newIsEntry = {id, s, gt};
                returnIsTable[i] = newIsEntry;
                break;
            case SEEN_STATUS_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &gt) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &s) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &sID) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                seen_status_entry newSeenEntry = {id, gt, s, sID};
                returnSeenTable[i] = newSeenEntry;
                break;
            case JOBS_KEY:
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &id) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                if(!cbor_value_is_integer(&entryValue) || cbor_value_get_int(&entryValue, &d) != CborNoError) {
                    return -1;
                }
                cbor_value_advance(&entryValue);
                jobs_entry newJobsEntry = {id, d};
                returnJobsTable[i] = newJobsEntry;
                break;
        }
        cbor_value_leave_container(&fieldsValue,&entryValue); // ]
    }

    cbor_value_leave_container(&wrapperValue, &fieldsValue); // ]	
    cbor_value_leave_container(&value, &wrapperValue); // ]	
    
    // TODO: Funtionen zum Integrieren aufrufen
    switch(tableType) {
            case TARGET_STATE_KEY:
                (void) returnTargetTable;
                break;
            case IS_STATE_KEY:
                (void) returnIsTable;
                break;
            case SEEN_STATUS_KEY:
                (void) returnSeenTable;
                break;
            case JOBS_KEY:
                (void) returnJobsTable;
                break;
            default:
                return -1;
    }

    return 0;
}

int target_state_table_to_cbor_many_test(target_state_entry table[], int package_size, cbor_buffer* buffer) {
    printf("Entered function\n");
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_TARGET_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        printf("%d + %d < %d\n", BASE_CBOR_BYTE_SIZE, CBOR_TARGET_STATE_MAX_BYTE_SIZE, package_size);
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    printf("This is the buffer: %p\n", buffer->buffer);
    while(table_index < 4) {
        printf("enterd while, i = %d, size = %d\n", table_index, size_of_current_cbor);
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        printf("Space = %p\n", space);
        printf("CBOR Index = %d\n", cbor_stream_index);
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 6); // Entry 2
        printf("while %d + %d < %d\n", size_of_current_cbor, CBOR_TARGET_STATE_MAX_BYTE_SIZE, package_size);
        while(size_of_current_cbor + CBOR_TARGET_STATE_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(true) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, table[table_index].timestamp);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
            printf("Entered one entry, size = %d\n", size_of_current_cbor);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int target_state_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_TARGET_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while(table_index < MAX_GATE_COUNT) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2
        while(size_of_current_cbor + CBOR_TARGET_STATE_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(true) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, target_state_entry_table[table_index].timestamp);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int is_state_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_IS_STATE_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while(table_index < MAX_GATE_COUNT) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, IS_STATE_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2
        while(size_of_current_cbor + CBOR_IS_STATE_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(true) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 3); // []
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].state);
                cbor_encode_int(&singleEntryEncoder, is_state_entry_table[table_index].gateTime);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int seen_status_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_SEEN_STATUS_MAX_BYTE_SIZE > package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while(table_index < MAX_GATE_COUNT) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, SEEN_STATUS_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2
        while(size_of_current_cbor + CBOR_SEEN_STATUS_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(true) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 4); // []
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].gateTime);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].senseMateID);
                cbor_encode_int(&singleEntryEncoder, seen_status_entry_table[table_index].status);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}

int jobs_table_to_cbor_many(int package_size, cbor_buffer* buffer) {
    // Assert: given package_size big enough
    if(BASE_CBOR_BYTE_SIZE + CBOR_JOBS_MAX_BYTE_SIZE> package_size) {
        printf("ASSERT failed. Size passed too small for cbor!\n");
        return -1;
    }

    int no_cbor_packages = 0;
    int cbor_stream_index = 0;
    int size_of_current_cbor = 0;
    int table_index = 0;
    while(table_index < MAX_GATE_COUNT) {
        CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
        uint8_t* space = (buffer->buffer) + (cbor_stream_index * sizeof(uint8_t));
        cbor_encoder_init(&encoder, space, sizeof(uint8_t) * package_size, 0);
        cbor_encoder_create_array(&encoder, &arrayEncoder, 2); // [
        cbor_encode_int(&arrayEncoder, JOBS_KEY); // Entry 1
        cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, MAX_GATE_COUNT); // Entry 2
        while(size_of_current_cbor + CBOR_JOBS_MAX_BYTE_SIZE < package_size) {
            //validate table entry
            if(true) {
                cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 2); // []
                cbor_encode_int(&singleEntryEncoder, jobs_entry_table[table_index].gateID);
                cbor_encode_int(&singleEntryEncoder, jobs_entry_table[table_index].done);
                cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]
            }
            table_index++;
            size_of_current_cbor = (uint8_t) cbor_encoder_get_buffer_size (&entriesEncoder, space);
        }
        cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
        cbor_encoder_close_container(&encoder, &arrayEncoder); // ]
        cbor_stream_index += size_of_current_cbor;
        buffer->package_size[no_cbor_packages] = (uint8_t) size_of_current_cbor;
        no_cbor_packages++;
        size_of_current_cbor = 0;
    }
    buffer->cbor_size = no_cbor_packages;
    return no_cbor_packages;
}