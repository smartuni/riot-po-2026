#ifndef SOUNDMODULE_H
#define SOUNDMODULE_H

void init_sound_module(void);

void downlink_reveived_sound(void);

void uplink_sent_sound(void);

void ble_reveived_sound(void);

void ble_sent_sound(void);

void startup_sound(void);

#endif // SOUNDMODULE_H