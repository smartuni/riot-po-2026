#include "displayDemo.h"
#include "ztimer.h"
#include <stdio.h>
#include <list.h>
#include "board.h"
#include "periph/gpio.h"
#include "ztimer.h"
#include "interrupts.h"
#include "menu.h"
#include "events_creation.h"




int main(void) {
    ztimer_sleep(ZTIMER_MSEC, 3000);
    init_interrupt();

    printf("Display demo started.\n");
    init_display();
    printf("Display initialized.\n");
    //display_demo();
    init_menu();
    set_current_meustate(INIT);
    refresh_display();
    init_event();
    printf("Menu initialized.\n");

    while (1)
    {
        ztimer_sleep(ZTIMER_MSEC, 200);
        //refresh_display();
    }
    
    printf("Display demo finished.\n");

    return 0;
}