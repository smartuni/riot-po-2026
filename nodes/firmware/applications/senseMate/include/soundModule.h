#ifndef SOUNDMODULE_H
#define SOUNDMODULE_H

#include "event.h"

extern event_queue_t sound_queue; // Event queue for sound events

extern event_t start_sound_event;
extern event_t downlink_sound_event;
extern event_t uplink_sound_event;
extern event_t ble_received_sound_event;
extern event_t ble_sent_sound_event;
extern event_t tables_news_sound_event;
extern event_t close_by_todo_sound_event;

/*
 * Initializes the sound module
 * This function should be called at the start of the program
 */
void init_sound_module(void);

/*
 * Plays the sound for downlink received
 * This function will post an event to the sound queue
 */
void downlink_reveived_sound(void);

/*
 * Plays the sound for uplink sent
 * This function will post an event to the sound queue
 */
void uplink_sent_sound(void);

/*
 * Plays the sound for BLE received
 * This function will post an event to the sound queue
 */
void ble_reveived_sound(void);

/*
 * Plays the sound for BLE sent
 * This function will post an event to the sound queue
 */
void ble_sent_sound(void);

/*
 * Plays the startup sound
 * This function will post an event to the sound queue
 */
void startup_sound(void);

#endif // SOUNDMODULE_H