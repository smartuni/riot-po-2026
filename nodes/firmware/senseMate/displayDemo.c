#include "include/displayDemo.h"
#include "u8g2.h"
#include "u8x8_riotos.h"
#include "ztimer.h"
#include "board.h"
#include <stdio.h>
#include <string.h>


#define DISPLAY_WIDTH (128)
#define DISPLAY_HEIGHT (64)
#define Y_OFFSET ((DISPLAY_HEIGHT/16)*3)
#define MIN_JOB_PRIO (1)
#define HIGH_JOB_PRIO (3)

u8g2_t u8g2;
u8x8_riotos_t user_data =
    {
        .device_index = I2C_DEVICE_IDX,
        .pin_cs = GPIO_UNDEF,
        .pin_dc = GPIO_UNDEF,
        .pin_reset = GPIO_UNDEF,
    };


void init_display(void){

    //puts("Initializing display.");
    U8G2_DISPLAY_INIT_FUNC(&u8g2, U8G2_R0, u8x8_byte_hw_i2c_riotos, u8x8_gpio_and_delay_riotos);

    u8g2_SetUserPtr(&u8g2, &user_data);
    u8g2_SetI2CAddress(&u8g2, I2C_DISPLAY_ADDR);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_helvR10_tf);
    u8g2_FirstPage(&u8g2);

    u8g2_SetFlipMode(&u8g2, 1); // flip mode
    u8g2_NextPage(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    //printf("Display initialized. --\n");
}


void display_menu_box(char *text, int num_after_text, bool use_num, int pos_x, int pos_y, int width, int height, bool selected){
    int relative_x = width/32; //expected 128/32 = 4
    int relative_y = height/7; //expected 28/7 = 4

    u8g2_DrawFrame(&u8g2, pos_x, pos_y, width, height);
    u8g2_DrawStr(&u8g2, pos_x+3*relative_x, pos_y+5*relative_y, text);

    if (use_num){
        char num_str[4];
        snprintf(num_str, sizeof(num_str), "%d", num_after_text);
        u8g2_DrawStr(&u8g2, pos_x+3*relative_x+u8g2_GetStrWidth(&u8g2, text)+relative_x/2, pos_y+5*relative_y, num_str);
        //printf("I want to display the number: %s, I got %d\n", num_str, num_after_text);
    }
    if(selected){
        int tri_top_y = pos_y + (2*relative_y);
        int tri_bottom_y = pos_y + (5*relative_y);

        //int tri_top_y = pos_y + 8;
        //int tri_bottom_y = pos_y + 20;

        u8g2_DrawTriangle(&u8g2, pos_x+relative_x, tri_bottom_y, pos_x+2*relative_x, (tri_bottom_y-tri_top_y)+pos_y+1, pos_x+relative_x, tri_top_y);
    }
    
}

void display_menu_header_box(char *text, int num_after_text, bool use_num, int pos_x, int pos_y, int width, int height){
    int relative_x = width/32; //expected 128/32 = 4
    int relative_y = height/7; //expected 28/7 = 4

    u8g2_DrawBox(&u8g2, pos_x, pos_y, width, height);
    u8g2_SetDrawColor(&u8g2, 0);
    
    //u8g2_SetFont(&u8g2, u8g2_font_helvR12_tf);

    u8g2_DrawStr(&u8g2, pos_x+3*relative_x, pos_y+5*relative_y, text);
    if (use_num){
        char num_str[4];
        snprintf(num_str, sizeof(num_str), "%d", num_after_text);
        u8g2_DrawStr(&u8g2, pos_x+3*relative_x + u8g2_GetStrWidth(&u8g2, text)+relative_x/2, pos_y+5*relative_y, num_str);
        
    }
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_helvR10_tf);

}

void display_menu_header(char *text, int num_after_text, bool use_num){
    do{
        display_menu_header_box(text, num_after_text, use_num, 0, (DISPLAY_HEIGHT/16), DISPLAY_WIDTH, (DISPLAY_HEIGHT/16)*7);
    }while (u8g2_NextPage(&u8g2));
}

void display_gate_state(int pos_x, int pos_y, bool gate_state_open, int relative_size){
    //state with Boxes only (X X):
    /*u8g2_DrawBox(&u8g2, pos_x, pos_y, 2*relative_size, 3*relative_size);
    u8g2_DrawBox(&u8g2, pos_x+5*relative_size, pos_y, 2*relative_size, 3*relative_size);
    if(!gate_state_open){
        u8g2_DrawBox(&u8g2, pos_x+2*relative_size, pos_y, 3*relative_size, 3*relative_size);
    }*/

    //state as "circuit-switch" (x/ x):
    u8g2_DrawBox(&u8g2, pos_x, pos_y+relative_size*2, 2*relative_size, relative_size);
    u8g2_DrawBox(&u8g2, pos_x+5*relative_size, pos_y+relative_size*2, 2*relative_size, relative_size);
    if(gate_state_open){
        for (int i = 0; i < relative_size; i++){
            u8g2_DrawLine(&u8g2, pos_x+2*relative_size+i, pos_y+relative_size*3-(relative_size-i), pos_x+3*relative_size+i, pos_y+i);
        }
    }else{
        u8g2_DrawBox(&u8g2, pos_x+2*relative_size, pos_y+relative_size*2, relative_size*3, relative_size);
    }

}

void display_gate_target(int pos_x, int pos_y, bool gate_match_target){
    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);
    if(gate_match_target){
        //u8g2_DrawGlyph(&u8g2, pos_x, pos_y, 0x2713); // check mark symbol
    }else{
        u8g2_DrawGlyph(&u8g2, pos_x, pos_y, 0x2715); // cross mark symbol
    }

    u8g2_SetFont(&u8g2, u8g2_font_helvR10_tf);
}

void display_job_prio(int pos_x, int pos_y, int job_prio){
    if(job_prio < MIN_JOB_PRIO){
        return;
    }
    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);
    if(job_prio >= HIGH_JOB_PRIO){
        u8g2_DrawGlyph(&u8g2, pos_x, pos_y, 0x23eb); // double up arrow symbol
    }else{
        u8g2_DrawGlyph(&u8g2, pos_x, pos_y, 0x23f6); // up arrow
    }

    u8g2_SetFont(&u8g2, u8g2_font_helvR10_tf);
    
}

void display_more_content(bool upper){
    if(upper){
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2, (DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2 + DISPLAY_WIDTH/32, (DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2 - DISPLAY_WIDTH/32, (DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
    }else{
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2, DISPLAY_HEIGHT-(DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2 + DISPLAY_WIDTH/32, DISPLAY_HEIGHT-(DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
        u8g2_DrawDisc(&u8g2, DISPLAY_WIDTH/2 - DISPLAY_WIDTH/32, DISPLAY_HEIGHT-(DISPLAY_HEIGHT/16)/2, DISPLAY_HEIGHT/64, U8G2_DRAW_ALL);
    }
}

void display_ordinary_menu(char *text, int num_after_text, bool use_num, bool upper, bool selected, bool more_content){
    do{

        if(upper){
            display_menu_box(text, num_after_text, use_num, 0, DISPLAY_HEIGHT/16, DISPLAY_WIDTH, (DISPLAY_HEIGHT/16)*7, selected);
        }else{
            display_menu_box(text, num_after_text, use_num, 0, (DISPLAY_HEIGHT/16)*8, DISPLAY_WIDTH, (DISPLAY_HEIGHT/16)*7, selected);    
        }

        if(more_content){
                display_more_content(upper);
        }

    } while (u8g2_NextPage(&u8g2));
}

void display_gate_menu_box(char* text, int num_after_text, bool upper, bool selected, bool gate_state_open, bool target_match, int job_prio, bool more_content){
    do{
        char my_text[11];
        u8g2_uint_t str_width = u8g2_GetStrWidth(&u8g2, text);
        if (str_width > 23*(DISPLAY_WIDTH/32)-3*(DISPLAY_WIDTH/32)){
            //int text_len = strlen(text);
            strncpy(my_text, text, 10);
            my_text[10] = '\0';
            text = my_text;
        }

        if(upper){
            display_menu_box(text, num_after_text, true, 0, DISPLAY_HEIGHT/16, DISPLAY_WIDTH, (DISPLAY_HEIGHT/16)*7, selected);
            display_gate_state(23*(DISPLAY_WIDTH/32), 4*DISPLAY_HEIGHT/16, gate_state_open, DISPLAY_HEIGHT/16);
            display_gate_target(28*(DISPLAY_WIDTH/32), 5*DISPLAY_HEIGHT/16, target_match);
            display_job_prio(18*(DISPLAY_WIDTH/32), 6*DISPLAY_HEIGHT/16, job_prio);

        }else{
            display_menu_box(text, num_after_text, true, 0, (DISPLAY_HEIGHT/16)*8, DISPLAY_WIDTH, (DISPLAY_HEIGHT/16)*7, selected);
            display_gate_state(23*(DISPLAY_WIDTH/32), (DISPLAY_HEIGHT/16)*11, gate_state_open, DISPLAY_HEIGHT/16);
            display_gate_target(28*(DISPLAY_WIDTH/32), (DISPLAY_HEIGHT/16)*12, target_match);
            display_job_prio(18*(DISPLAY_WIDTH/32), (DISPLAY_HEIGHT/16)*13, job_prio);
        }

        if(more_content){
                display_more_content(upper);
        }

    } while (u8g2_NextPage(&u8g2));
}


void new_page(void){
    u8g2_FirstPage(&u8g2);
}