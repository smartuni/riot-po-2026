#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "tables.h"

#define TEST_TABLE_TO_CBOR

int main(void) 
{
    
/**
 * Test for testing table struct to cbor functions
 * signature of function needs to be changed for test to work
*/
#ifdef TEST_TABLE_TO_CBOR
    puts("Starting CBOR Test");

    target_state_entry e1 = {4, 2, 2000};
    target_state_entry e2 = {5, 0, 2001};
    target_state_entry test[] = {e1, e2};
    puts("Made test object");

    cbor_buffer* buf = (cbor_buffer*) malloc(sizeof(uint8_t) * 120);
    buf->buffer = (uint8_t*) malloc(sizeof(uint8_t) * 100);
    int result = target_state_table_to_cbor_test(test, buf);
    puts("Used function");

    printf("Result is: %d\n", result);
    printf("Size of cbor package: %d\n", buf->cbor_size);
    for(int i = 0; i < 20; i++) {
        printf("%x ", buf->buffer[i]);
    }
    printf("\n");

    puts("End of Package");

    free(buf->buffer);
    free(buf);
#endif
    return 0;
}