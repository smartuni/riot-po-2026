#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_GATE_ICON_OPEN_8X9
#define LV_ATTRIBUTE_IMG_GATE_ICON_OPEN_8X9
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_GATE_ICON_OPEN_8X9 uint8_t gate_icon_open_8x9_map[] = {
  0x00, 0x00, 0x00, 0x00, 	/*Color of index 0*/
  0xfe, 0xfe, 0xfe, 0xfe, 	/*Color of index 1*/

  0x89, 
  0x8d, 
  0x8f, 
  0x8f, 
  0x8f, 
  0x8f, 
  0x87, 
  0x83, 
  0x81, 
};

const lv_img_dsc_t gate_icon_open_8x9 = {
  .header.cf = LV_IMG_CF_INDEXED_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 8,
  .header.h = 9,
  .data_size = 17,
  .data = gate_icon_open_8x9_map,
};
