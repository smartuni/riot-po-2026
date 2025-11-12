/*******************************************************************************
 * Size: 9 px
 * Bpp: 1
 * Opts: --bpp 1 --size 9 --no-compress --stride 1 --align 1 --font pixelmix-micro-regular.ttf --range 32-127 --format lvgl -o micro_regular.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef MICRO_REGULAR
#define MICRO_REGULAR 1
#endif

#if MICRO_REGULAR

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xf4,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x53, 0xf5, 0x14, 0xfd, 0x40,

    /* U+0024 "$" */
    0x4b, 0xc4, 0xf, 0x40,

    /* U+0025 "%" */
    0xc7, 0x42, 0x0, 0x5e, 0x70,

    /* U+0026 "&" */
    0x42, 0x84, 0x0, 0xa6, 0x46, 0x40,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x6a, 0x90,

    /* U+0029 ")" */
    0x95, 0x60,

    /* U+002A "*" */
    0xa1, 0x50,

    /* U+002B "+" */
    0x4b, 0xa0,

    /* U+002C "," */
    0xc0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x29, 0x25, 0x0,

    /* U+0030 "0" */
    0x69, 0x99, 0x96,

    /* U+0031 "1" */
    0xfc,

    /* U+0032 "2" */
    0x69, 0x24, 0x4f,

    /* U+0033 "3" */
    0xc5, 0x3, 0x80,

    /* U+0034 "4" */
    0xb7, 0x92, 0x40,

    /* U+0035 "5" */
    0xf3, 0x93, 0xc0,

    /* U+0036 "6" */
    0xf3, 0xdb, 0xc0,

    /* U+0037 "7" */
    0xe4, 0x94, 0x80,

    /* U+0038 "8" */
    0xf7, 0xdb, 0xc0,

    /* U+0039 "9" */
    0xf7, 0x93, 0xc0,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x98,

    /* U+003C "<" */
    0x2a, 0x4, 0x40,

    /* U+003D "=" */
    0xe3, 0x80,

    /* U+003E ">" */
    0x88, 0x85, 0x0,

    /* U+003F "?" */
    0xdc, 0x20,

    /* U+0040 "@" */
    0x7d, 0x6, 0xcd, 0x8b, 0xd0, 0x1f, 0x0,

    /* U+0041 "A" */
    0xf7, 0xdb, 0x40,

    /* U+0042 "B" */
    0xf7, 0x5b, 0xc0,

    /* U+0043 "C" */
    0xf2, 0x49, 0xc0,

    /* U+0044 "D" */
    0xd6, 0xdb, 0x80,

    /* U+0045 "E" */
    0xf3, 0xc9, 0xc0,

    /* U+0046 "F" */
    0xf3, 0xc9, 0x0,

    /* U+0047 "G" */
    0xf8, 0xb9, 0x9f,

    /* U+0048 "H" */
    0x99, 0xf9, 0x99,

    /* U+0049 "I" */
    0xe9, 0x25, 0xc0,

    /* U+004A "J" */
    0x24, 0x9b, 0xc0,

    /* U+004B "K" */
    0x9a, 0xc8, 0xa9,

    /* U+004C "L" */
    0x92, 0x49, 0xc0,

    /* U+004D "M" */
    0x8e, 0xeb, 0x18, 0xc4,

    /* U+004E "N" */
    0x8e, 0x6b, 0x19, 0xc4,

    /* U+004F "O" */
    0xf9, 0x99, 0x9f,

    /* U+0050 "P" */
    0xf7, 0xc9, 0x0,

    /* U+0051 "Q" */
    0xf2, 0x49, 0x24, 0x93, 0xc0, 0x40,

    /* U+0052 "R" */
    0xf7, 0xe9, 0x40,

    /* U+0053 "S" */
    0xf1, 0x13, 0xc0,

    /* U+0054 "T" */
    0xe9, 0x24, 0x80,

    /* U+0055 "U" */
    0x99, 0x99, 0x9f,

    /* U+0056 "V" */
    0x99, 0x99, 0x66,

    /* U+0057 "W" */
    0x86, 0x1a, 0x41, 0x5d, 0x40,

    /* U+0058 "X" */
    0x85, 0x42, 0x0, 0x52, 0x10,

    /* U+0059 "Y" */
    0x85, 0x42, 0x8, 0x20, 0x80,

    /* U+005A "Z" */
    0xfc, 0x42, 0x10, 0x43, 0xf0,

    /* U+005B "[" */
    0xea, 0xb0,

    /* U+005C "\\" */
    0x89, 0x24, 0x40,

    /* U+005D "]" */
    0xd5, 0x70,

    /* U+005E "^" */
    0xff, 0xff, 0xc0,

    /* U+005F "_" */
    0xe0,

    /* U+0060 "`" */
    0xff, 0xff, 0xc0,

    /* U+0061 "a" */
    0x69, 0x91, 0x70,

    /* U+0062 "b" */
    0x8e, 0x99, 0x8e,

    /* U+0063 "c" */
    0x72, 0x6,

    /* U+0064 "d" */
    0x17, 0x99, 0x17,

    /* U+0065 "e" */
    0x69, 0xe8, 0x86,

    /* U+0066 "f" */
    0x6e, 0xa0,

    /* U+0067 "g" */
    0x79, 0x91, 0x71, 0x60,

    /* U+0068 "h" */
    0x93, 0x4b, 0x40,

    /* U+0069 "i" */
    0xbc,

    /* U+006A "j" */
    0x20, 0x92, 0x6a,

    /* U+006B "k" */
    0x92, 0xcd, 0x40,

    /* U+006C "l" */
    0xfc,

    /* U+006D "m" */
    0x52, 0x9a, 0x69, 0xa4,

    /* U+006E "n" */
    0xe9, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x90, 0x60,

    /* U+0070 "p" */
    0x69, 0x98, 0xe8, 0x80,

    /* U+0071 "q" */
    0x62, 0x49, 0x4, 0x70, 0x41, 0xc0,

    /* U+0072 "r" */
    0x6a, 0x80,

    /* U+0073 "s" */
    0x71, 0x3, 0x80,

    /* U+0074 "t" */
    0x5d, 0x24, 0x80,

    /* U+0075 "u" */
    0x99, 0x99, 0x60,

    /* U+0076 "v" */
    0xb6, 0xd4,

    /* U+0077 "w" */
    0x86, 0x1a, 0x69, 0x50,

    /* U+0078 "x" */
    0x96, 0x60, 0x90,

    /* U+0079 "y" */
    0x99, 0x99, 0x71, 0x70,

    /* U+007A "z" */
    0xf9, 0x18, 0x8f, 0x80,

    /* U+007B "{" */
    0xff, 0xff, 0xc0,

    /* U+007C "|" */
    0xfc,

    /* U+007D "}" */
    0xff, 0xff, 0xc0,

    /* U+007E "~" */
    0x5a
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 18, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 36, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 72, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 3, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 72, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 12, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 23, .adv_w = 36, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 24, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 72, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 30, .adv_w = 72, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 32, .adv_w = 36, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 34, .adv_w = 36, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 36, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 36, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 36, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 72, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 73, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 126, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 85, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 108, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 108, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 141, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 90, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 72, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 54, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 90, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 210, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 36, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 217, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 36, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 108, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 90, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 90, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 90, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 235, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 241, .adv_w = 54, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 90, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 72, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 108, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 90, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 90, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 265, .adv_w = 90, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 36, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 90, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t micro_regular = {
#else
lv_font_t micro_regular = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if MICRO_REGULAR*/
