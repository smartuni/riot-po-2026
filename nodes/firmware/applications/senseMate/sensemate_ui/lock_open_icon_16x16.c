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

#ifndef LV_ATTRIBUTE_IMG_LOCK_OPEN_ICON_16X16
#define LV_ATTRIBUTE_IMG_LOCK_OPEN_ICON_16X16
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_LOCK_OPEN_ICON_16X16 uint8_t lock_open_icon_16x16_map[] = {
  0xff, 0xff, 0xff, 0xff, 	/*Color of index 0*/
  0x00, 0x00, 0x00, 0x0e, 	/*Color of index 1*/

  0xff, 0xff, 
  0xff, 0x83, 
  0xff, 0x01, 
  0xfe, 0x38, 
  0xfe, 0x7c, 
  0xfe, 0x7c, 
  0xfe, 0x7c, 
  0xfe, 0x7c, 
  0x00, 0x1f, 
  0x00, 0x1f, 
  0x0e, 0x1f, 
  0x0e, 0x1f, 
  0x04, 0x1f, 
  0x04, 0x1f, 
  0x00, 0x1f, 
  0x00, 0x1f, 
};

const lv_img_dsc_t lock_open_icon_16x16 = {
  .header.cf = LV_IMG_CF_INDEXED_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 16,
  .header.h = 16,
  .data_size = 40,
  .data = lock_open_icon_16x16_map,
};
