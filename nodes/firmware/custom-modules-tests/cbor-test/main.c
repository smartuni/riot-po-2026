#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "tables.h"
#include <string.h>
#include "cbor.h"

//#define TEST_TABLE_TO_CBOR
#define TEST_CBOR_TO_TABLE
//#define TEST_TABLE_TO_MANY_CBOR

int test_target_to_cbor(cbor_buffer* buf) {
    puts("Starting encode CBOR Test");

    target_state_entry e1 = {4, 2, 2000};
    target_state_entry e2 = {5, 0, 2801};
    target_state_entry e3 = {3, 1, 2051};
    target_state_entry e4 = {8 , 0, 2401};
    
    target_state_entry test[] = {e1, e2, e3, e4};
    puts("Made test object");

    int result = target_state_table_to_cbor_test(test, buf);
    puts("Used function");

    printf("Result is: %d\n", result);
    printf("Size of cbor package: %d\n", buf->cbor_size);
    for(int i = 0; i < buf->cbor_size; i++) {
        printf("%x ", buf->buffer[i]);
    }
    printf("\n");

    puts("End of Package");

    return 0;
}

int test_cbor_to_target(cbor_buffer* buf) {
    puts("Starting decode CBOR Test");

    CborEncoder encoder, arrayEncoder, entriesEncoder, singleEntryEncoder;
    cbor_encoder_init(&encoder, buf->buffer, sizeof(uint8_t) * 100, 0);
    cbor_encoder_create_array(&encoder, &arrayEncoder, 3); // [
    cbor_encode_int(&arrayEncoder, TARGET_STATE_KEY); // Entry 1
    cbor_encode_int(&arrayEncoder, 2008); // Entry 2
    cbor_encoder_create_array(&arrayEncoder, &entriesEncoder, 2); // Entry 3

    cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 2); // []
    cbor_encode_int(&singleEntryEncoder, 13);
    cbor_encode_int(&singleEntryEncoder, 1);
    cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]

    cbor_encoder_create_array(&entriesEncoder, &singleEntryEncoder, 2); // []
    cbor_encode_int(&singleEntryEncoder, 7);
    cbor_encode_int(&singleEntryEncoder, 0);
    cbor_encoder_close_container(&entriesEncoder, &singleEntryEncoder); // ]

    cbor_encoder_close_container(&arrayEncoder, &entriesEncoder); // ]
    cbor_encoder_close_container(&encoder, &arrayEncoder); // ]

    buf->cbor_size = (uint8_t) cbor_encoder_get_buffer_size (&encoder, buf->buffer);

    puts("Entering function");
    cbor_to_table_test(buf, 0);

    puts("Finished test cbor to table");

    return 0;
}

int test_targ_to_cbor_many(cbor_buffer* buf) {
    puts("Starting encode CBOR many Test");
    target_state_entry e1 = {4, 2, 2000};
    target_state_entry e2 = {5, 0, 2801};
    target_state_entry e3 = {3, 1, 2051};
    target_state_entry e4 = {8, 0, 2401};
    target_state_entry test[] = {e1, e2, e3, e4};

    puts("Made test object");        

    int no_pack = target_state_table_to_cbor_many_test(test, 22, buf);
    puts("Used function");

    printf("Return is: %d\n", no_pack);
    printf("Size of cbor packages: %d\n", buf->cbor_size);
    for(int i = 0; i < no_pack; i++) {
        printf("%d ", buf->package_size[i]);
    }
    printf("\n");
    printf("CBOR string: ");
    for(int j = 0; j < no_pack * 17; j++) {
        printf("%x ", buf->buffer[j]);
    }
    printf("\n");
    puts("End of Test converting table to many cbor streams");

    return 0;
}

int main(void) 
{
    ztimer_sleep(ZTIMER_SEC, 5);
    
    /**
     * Test for testing table functions
    */
    cbor_buffer* buf = (cbor_buffer*) malloc(sizeof(uint8_t) * 120);
    buf->buffer = (uint8_t*) malloc(sizeof(uint8_t) * 100);
    buf->package_size = (uint8_t*) malloc(sizeof(uint8_t) * 100);

    test_target_to_cbor(buf);

    //test_cbor_to_target(&buf);

    // test_targ_to_cbor_many(&buf);

    free(buf->buffer);
    free(buf->package_size);
    free(buf);
    return 0;
}