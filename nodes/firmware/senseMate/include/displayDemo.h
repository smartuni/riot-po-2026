#ifndef _MY_DISPLAY_DEMO_H
#define _MY_DISPLAY_DEMO_H
#include <stdbool.h>

/*
 * Initializes the display
*/
extern void init_display(void);

//extern void display_menu(char *entry01 /*, int entry01_len*/, char *entry02/*, int entry02_len*/, bool aboveMore, bool belowMore, int selected);

/**
 * Displays the gate menu box
 * @param text The text to display
 * @param num_after_text The number of the gate
 * @param upper If true, the box is displayed in the upper half of the screen, false for the lower half
 * @param selected If true, the box is displayed as selected
 * @param gate_state_open If true, the gate state is shown as open
 * @param target_match If false, the target state is shown as not matching
 * @param job_prio The job priority to display
 * @param more_content If true, it's shown that there is more content to display, for upper above, for not upper below
 */
extern void display_gate_menu_box(char* text, int num_after_text, bool upper, bool selected, bool gate_state_open, bool target_match, int job_prio, bool more_content);


/**
 * Displays the ordinary menu
 * @param text The text to display
 * @param num_after_text The number of the gate
 * @param use_num If true, the number is displayed after the text
 * @param upper If true, the box is displayed in the upper half of the screen, false for the lower half
 * @param selected If true, the box is displayed as selected
 * @param more_content If true, it's shown that there is more content to display, for upper above, for not upper below
 */
extern void display_ordinary_menu(char *text, int num_after_text, bool use_num, bool upper, bool selected, bool more_content);

extern void display_menu_header(char *text, int num_after_text, bool use_num);

extern void new_page(void);

#endif