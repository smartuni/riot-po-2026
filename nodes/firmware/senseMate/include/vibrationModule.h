#ifndef VIBRATION_MODULE_H
#define VIBRATION_MODULE_H

/*
 * Initializes the vibration module
 * This function should be called at the start of the program
 */
void init_vibration_module(void);

/*
 * Starts the vibration
 * This function will set the GPIO pin to start the vibration
 */
void start_vibration(void);

/*
 * Stops the vibration
 * This function will clear the GPIO pin to stop the vibration
 */
void stop_vibration(void);

#endif // VIBRATION_MODULE_H