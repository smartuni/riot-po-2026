#include "header/event_creation.h"

// ID of Gatemate
int id = GATE_ID;
uint8_t time = 0;


gpio_t led0 = GPIO_PIN(1, 10); // TODO REMOVE LATER #1
gpio_mode_t led0_mode = GPIO_OUT; // TODO REMOVE LATER #1


// TABLE
is_state_entry table_entry;

uint8_t event_status = 0;

// Debounce
bool event_accepted = true;
event_timeout_t reactivate;

void event_handlerA0(event_t *event);
void event_handler_reactivate(event_t *event);
event_t eventA0 = { .handler = event_handlerA0 };
event_t event_reactivate = { .handler = event_handler_reactivate };



void init_event(void){
    gpio_init(led0, led0_mode);
    gpio_clear(led0);
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
    (void) event;   /* Not used */
    event_accepted = true; // Allow the event handler to be called again

    puts("sending");
    // UPDATE TABLE
    table_entry.gateID = GATE_ID;
    table_entry.state = event_status;
    table_entry.gateTime = time++;

    // if (TABLE_SUCCESS == set_is_state_entry()){
    if (0 == set_is_state_entry(&table_entry)){
        // CALL UPDATE
        event_post(&lorawan_queue, &send_event );
    } 
    
}

void event_handlerA0(event_t *event)
{
    (void) event;   /* Not used */
    if(event_accepted){
        event_accepted = false; // Prevent further calls until reset
              
        puts("Set timer");
        gpio_set(led0);
        event_timeout_set(&reactivate, DEBOUNCE_TIME); // Set a timeout to allow reactivation
       
    }else{
        puts("Update timer");
        event_timeout_set(&reactivate, DEBOUNCE_TIME); // Set a timeout to allow reactivation        
        gpio_clear(led0);
    }
    
}

void update_status(uint8_t newStatus){
    event_status = newStatus;
}