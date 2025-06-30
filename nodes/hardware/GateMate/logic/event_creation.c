#include "header/event_creation.h"

// ID of Gatemate
int id = GATE_ID;

gpio_t led0 = GPIO_PIN(1, 10); // TODO REMOVE LATER #1
gpio_mode_t led0_mode = GPIO_OUT; // TODO REMOVE LATER #1


// TABLE
is_state_entry table_entry;

uint8_t event_status = 0;
uint32_t timestamp = 0;

// Debounce
bool event_accepted = true;
event_timeout_t reactivate;

void event_handlerA0(event_t *event);
void event_handler_reactivate(event_t *event);
event_t eventA0 = { .handler = event_handlerA0 };
event_t event_reactivate = { .handler = event_handler_reactivate };



void init_event(void){
    gpio_init(led0, led0_mode);
    gpio_set(led0);
    event_timeout_ztimer_init(&reactivate, ZTIMER_MSEC, EVENT_PRIO_HIGHEST, &event_reactivate);
}


void event_callback (void *arg)
{
    (void) arg; /* the argument is not used */
    ztimer_sleep(ZTIMER_MSEC, 500); // Wait for 0.5 second before accepting the next event
    event_accepted = true; // Allow the event handler to be called again  
}

void event_handler_reactivate(event_t *event) 
{
    gpio_set(led0);
    (void) event;   /* Not used */
    event_accepted = true; // Allow the event handler to be called again

    // UPDATE TABLE
    table_entry.gateID = GATE_ID;
    table_entry.state = event_status;
    table_entry.gateTime = timestamp++;
    puts("In event reactivate handler");
    if( TABLE_UPDATED == set_is_state_entry(&table_entry)){
        puts("In event reactivate handler IF");
        // TEST
        is_state_entry test_table_entry;
        if (!get_is_state_entry(GATE_ID, &test_table_entry)) {
            puts("In event reactivate handler GET");
            printf("TEST ENTRY Time %d\n", test_table_entry.gateTime);
            printf("TEST ENTRY State %d\n", test_table_entry.state);
        } else {
            puts("test failed");
        } 
    } else {
        puts("writing to table failed!");
    }
    
    
}

void event_handlerA0(event_t *event)
{
    (void) event;   /* Not used */
    gpio_clear(led0); 
    if(event_accepted){
        event_accepted = false; // Prevent further calls until reset
        event_timeout_set(&reactivate, DEBOUNCE_TIME); // Set a timeout to allow reactivation
    }else{
        event_timeout_set(&reactivate, DEBOUNCE_TIME); // Set a timeout to allow reactivation    
           
    }
    
}

void update_status(uint8_t newStatus){
    event_status = newStatus;
}

void setTimestamp(int newTimestamp) {
    timestamp = newTimestamp;
}

int getTimestamp(void) {
    return timestamp;
}