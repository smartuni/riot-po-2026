#include <stdio.h>
#include "ztimer.h"
#include "../../ble/ble.h"

int main(void)
{
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_CPU);

    return 0;
}