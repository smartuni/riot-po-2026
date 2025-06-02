#ifndef _MY_DISPLAY_DEMO_H
#define _MY_DISPLAY_DEMO_H
#include <stdbool.h>

extern void display_demo(void);

extern void init_display(void);

extern void display_menu(char *entry01 /*, int entry01_len*/, char *entry02/*, int entry02_len*/, bool aboveMore, bool belowMore, int selected);

extern void display_gate_menu_box(char* text, bool upper, bool selected, bool gate_state_open, bool more_content);

extern void display_ordinary_menu(char *text, bool upper, bool selected, bool more_content);

extern void display_menu_header(char *text);

extern void new_page(void);

#endif