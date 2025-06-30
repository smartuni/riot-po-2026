#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "tables.h"
#include <string.h>

#define TEST_TABLE_TO_CBOR
#define TEST_CBOR_TO_TABLE
//#define TEST_TABLE_TO_MANY_CBOR

int main(void) 
{
    ztimer_sleep(ZTIMER_SEC, 5);
    
    /**
     * Test for testing table functions
    */
    cbor_buffer* buf = (cbor_buffer*) malloc(sizeof(uint8_t) * 120);
    buf->buffer = (uint8_t*) malloc(sizeof(uint8_t) * 100);
    buf->package_size = (uint8_t*) malloc(sizeof(uint8_t) * 100);

    #ifdef TEST_TABLE_TO_CBOR
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
    #endif

    /**
     * Test for testing cbor to table
     * uses buf created using target_state_table_to_cbor_test so the test above must run beforehand
    */
    #ifdef TEST_CBOR_TO_TABLE
        puts("Starting decode CBOR Test");

        puts("Entering function");
        cbor_to_table_test(buf);

        puts("Finished test cbor to table");
    #endif

    #ifdef TEST_TABLE_TO_MANY_CBOR
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

    #endif

    free(buf->buffer);
    free(buf->package_size);
    free(buf);
    return 0;
}