#include "health_monitor.h"

health_monitor_t* health_monitor_init(void) {
    health_monitor_t* monitor = malloc(sizeof(health_monitor_t));
    if (monitor == NULL) {
        
        return NULL;
    }
    monitor->placeholder = 0; //TODO remove placeholder
    return monitor;
}

int health_monitor_start(health_monitor_t* monitor) {
    if (monitor == NULL) {
        
        return -1;
    }
    printf(" health monitor is started.\n");
    return 0;
}

int health_monitor_stop(health_monitor_t* monitor) {
    if (monitor == NULL) {
        
        return -1;
    }
    printf(" health monitor is stopped.\n");
    return 0;
}