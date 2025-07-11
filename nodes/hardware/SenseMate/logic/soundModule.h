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


void init_sound_module(void);

void downlink_reveived_sound(void);

void uplink_sent_sound(void);

void ble_reveived_sound(void);

void ble_sent_sound(void);

void startup_sound(void);

#endif // SOUNDMODULE_H