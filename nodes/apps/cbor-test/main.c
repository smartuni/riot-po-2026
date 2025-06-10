#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "tables.h"

int main(void) 
{
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("Starting CBOR Test");

    target_state_entry e1 = {4, 2, 2000};
    target_state_entry e2 = {5, 0, 2001};
    target_state_entry test[] = {e1, e2};
    puts("Made test object");

    cbor_buffer* buf = (cbor_buffer*) malloc(sizeof(uint8_t) * 120);
    int result = target_state_table_to_cbor(test, buf);
    puts("Used function");

    printf("Result is: %d", result);
    printf("Size of cbor package: %d", buf->cbor_size);
    for(int i = 0; i < buf->cbor_size; i++) {
        printf("%d ", buf->buffer[i]);
    }

    puts("End of Package");

    return 0;
}