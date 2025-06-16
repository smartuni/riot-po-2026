#ifndef _MY_MENU_H
#define _MY_MENU_H
#include <stdbool.h>
#include <stdint.h>

#define INIT 0
#define UP 1
#define DOWN 2
#define SELECT 3

typedef struct menu_header_t menu_header_t;

typedef struct{
    char *text;
    int id;
    uint8_t state;
    menu_header_t *next;
} menu_entry_t;

typedef struct menu_header_t{
    char *text;
    int id;
    menu_entry_t *first_entry;
    int num_entries;
} menu_header_t;

extern void init_menu(void);

extern void set_current_meustate(int input);

extern void refresh_display(void);

extern void refresh_menu(void);

#endif