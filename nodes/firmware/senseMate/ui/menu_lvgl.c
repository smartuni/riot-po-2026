#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl_riot.h"
#include "disp_dev.h"
#include "lvgl/src/extra/widgets/list/lv_list.h"
#include "lvgl/src/extra/widgets/tileview/lv_tileview.h"
#include "lvgl/src/core/lv_theme.h"
#include "lvgl/src/extra/themes/mono/lv_theme_mono.h"
#include "periph/gpio.h"

#define THUMBWHEEL_PIN_DOWN   GPIO_PIN(0, 4)
#define THUMBWHEEL_PIN_SELECT GPIO_PIN(0, 5)
#define THUMBWHEEL_PIN_UP     GPIO_PIN(0, 28)

/* Must be lower than LVGL_INACTIVITY_PERIOD_MS for autorefresh */
#define REFR_TIME           200

lv_timer_t *refr_task;
extern lv_font_t helvetica_light_12;
extern lv_font_t helvetica10;
extern const lv_img_dsc_t gate_icon;
extern const lv_img_dsc_t tasks_icon_32;
extern const lv_img_dsc_t person_icon_32;
extern const lv_img_dsc_t bluetooth_icon_7x11;

static lv_style_t style_noborder;
static lv_indev_drv_t indev_drv;
static lv_group_t *nav_group;
static lv_indev_t *indev;
static lv_obj_t * tileview;

void wakeup_task(lv_timer_t *param)
{
    (void)param;
    /* Force a wakeup of lvgl when each task is called: this ensures an activity
       is triggered and wakes up lvgl during the next LVGL_INACTIVITY_PERIOD ms */
    lvgl_wakeup();
}

void encoder_with_keys_read(lv_indev_drv_t * drv, lv_indev_data_t*data){
  static lv_key_t last_key = LV_KEY_ENTER;
  (void)drv;
  data->state = LV_INDEV_STATE_PRESSED;
  if (!gpio_read(THUMBWHEEL_PIN_DOWN)) {
    data->key = LV_KEY_RIGHT;
  } else if (!gpio_read(THUMBWHEEL_PIN_SELECT)) {
    data->key = LV_KEY_ENTER;
  } else if (!gpio_read(THUMBWHEEL_PIN_UP)) {
    data->key = LV_KEY_LEFT;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = last_key;
  }
  last_key = data->key;
}

void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    (void)obj;
    if(code == LV_EVENT_CLICKED) {
        printf("CLICKED %d@%p\n", code, obj);
    } else if(code == LV_EVENT_FOCUSED) {
        printf("FOCUSED: %s\n", lv_label_get_text(lv_obj_get_child(obj, 0)));
    }
}

void create_list_menu(lv_obj_t *parent, lv_group_t *grp)
{
    lv_obj_t *list1 = lv_list_create(parent);
    lv_obj_set_size(list1, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list1);
    lv_obj_add_style(list1, &style_noborder, 0); 

    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Closeby Gates");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "All Gates");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Jobs");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    
    btn = lv_list_add_btn(list1, LV_SYMBOL_WARNING, "Messages");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "Settings");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_add_flag(list1, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_OFF);
}

lv_obj_t *add_badged_icon(lv_obj_t *parent, const void *img_src, const char *badge_str) {
    lv_obj_t *badged_icon_cont = lv_obj_create(parent);
    lv_obj_add_style(badged_icon_cont, &style_noborder, 0); 
    lv_obj_set_size(badged_icon_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(badged_icon_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(badged_icon_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *icon = lv_img_create(badged_icon_cont);

    lv_img_set_src(icon, img_src);
    lv_obj_t *badge_lbl = lv_label_create(badged_icon_cont);
    lv_obj_set_style_radius(badge_lbl, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(badge_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(badge_lbl, badge_str);
    lv_obj_set_height(badge_lbl, 14);
    lv_obj_set_width(badge_lbl, 14);
    lv_obj_center(badge_lbl);
    lv_obj_set_style_text_align(badge_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(badge_lbl, icon, LV_ALIGN_BOTTOM_RIGHT, 4, 4);
    lv_style_value_t local_style_val = { .color = lv_color_white() };
    lv_obj_set_local_style_prop(badge_lbl, LV_STYLE_BG_COLOR, local_style_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    local_style_val.num = LV_OPA_COVER;
    lv_obj_set_local_style_prop(badge_lbl, LV_STYLE_BG_OPA, local_style_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    local_style_val.color = lv_color_black();
    lv_obj_set_local_style_prop(badge_lbl, LV_STYLE_TEXT_COLOR, local_style_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    local_style_val.color = lv_color_black();
    lv_obj_set_local_style_prop(badge_lbl, LV_STYLE_OUTLINE_COLOR, local_style_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    local_style_val.num = 3;
    lv_obj_set_local_style_prop(badge_lbl, LV_STYLE_OUTLINE_WIDTH, local_style_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    return badge_lbl;
}

lv_obj_t *add_header_label(lv_obj_t *parent, const char *txt)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, txt);
    lv_obj_center(lbl);
    lv_obj_set_style_pad_all(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return lbl;
}

void create_dashboard(lv_obj_t *parent, lv_group_t *grp)
{
    lv_obj_t *root_cont = lv_obj_create(parent);
    lv_obj_set_size(root_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(root_cont);
    lv_obj_set_flex_flow(root_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_cont, 1, LV_PART_MAIN);
    lv_obj_add_style(root_cont, &style_noborder, 0); 
    lv_obj_add_flag(root_cont, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    /* if this flag is not cleared, a click enters this container
     * and appears to lock the containig tile, which can only be released
     * with along press */
    lv_obj_clear_flag(root_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_group_add_obj(grp, root_cont);

    lv_obj_set_style_pad_all(root_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t *header_cont = lv_obj_create(root_cont);
    lv_obj_add_style(header_cont, &style_noborder, 0); 
    lv_obj_set_size(header_cont, LV_PCT(100), LV_PCT(20));
    lv_obj_set_flex_flow(header_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(header_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(header_cont, LV_SCROLLBAR_MODE_OFF);
    
    /* left part of the header symbols */
    lv_obj_t * icon = lv_img_create(header_cont);
    lv_img_set_src(icon, &bluetooth_icon_7x11);
    
    lv_obj_t *lora_lbl = add_header_label(header_cont, "LoRa");
    lv_obj_t *usb_lbl = add_header_label(header_cont, LV_SYMBOL_USB);
    
    /* empty space between left and right part of the header/status bar */
    lv_obj_t * header_pad;
    header_pad = lv_obj_create(header_cont);
    lv_obj_set_height(header_pad, LV_PCT(100));
    lv_obj_set_flex_grow(header_pad, 1);
    lv_obj_add_style(header_pad, &style_noborder, 0); 
    lv_obj_set_style_pad_all(header_pad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* right part of the header symbols */
    lv_obj_t *alert_lbl = add_header_label(header_cont, LV_SYMBOL_BELL);
    lv_obj_t *bat_lbl = add_header_label(header_cont, LV_SYMBOL_BATTERY_FULL);

    lv_obj_t *dash_cont = lv_obj_create(root_cont);
    lv_obj_set_width(dash_cont, LV_PCT(100));
    lv_obj_set_flex_grow(dash_cont, 1);
    //lv_obj_set_flex_flow(dash_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_flow(dash_cont, LV_FLEX_FLOW_ROW);
    lv_obj_add_style(dash_cont, &style_noborder, 0); 
    lv_style_value_t bord_top_val = { .num = LV_BORDER_SIDE_TOP };
    lv_obj_set_local_style_prop(dash_cont, LV_STYLE_BORDER_SIDE, bord_top_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    bord_top_val.num = 1;
    lv_obj_set_local_style_prop(dash_cont, LV_STYLE_BORDER_WIDTH, bord_top_val, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_align(dash_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);
    
    lv_obj_t *badge_lbl_gates = add_badged_icon(dash_cont, &gate_icon, "5");
    lv_obj_t *badge_lbl_tasks = add_badged_icon(dash_cont, &tasks_icon_32, "3");
    lv_obj_t *badge_lbl_persons = add_badged_icon(dash_cont, &person_icon_32, "2");
    
    //TODO: store label references for status/visibility updates
    (void)alert_lbl;
    (void)bat_lbl;
    (void)lora_lbl;
    (void)usb_lbl;
    (void)badge_lbl_gates;
    (void)badge_lbl_tasks;
    (void)badge_lbl_persons;
}

int sensemate_menu_init(void)
{

    gpio_init(THUMBWHEEL_PIN_DOWN, GPIO_IN_PU);
    gpio_init(THUMBWHEEL_PIN_SELECT, GPIO_IN_PU);
    gpio_init(THUMBWHEEL_PIN_UP, GPIO_IN_PU);
    
    /* create a new input device */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_with_keys_read;
    /*Register the driver in LVGL and save the created input device object*/
    indev = lv_indev_drv_register(&indev_drv);
    
    /* Enable backlight */
    disp_dev_backlight_on();

    lv_theme_t *theme = lv_theme_mono_init(lv_disp_get_default(), true, &helvetica_light_12);
    lv_disp_set_theme(lv_disp_get_default(), theme); 
    
    /* Initial navigation group, key inputs will cycle the focus
     * through all elements in this group */
    nav_group = lv_group_create();
    lv_group_set_wrap(nav_group, true);
    /* use the custom input device for navigating through elements in this group */
    lv_indev_set_group(indev, nav_group);
    
    lv_style_init(&style_noborder);
    lv_style_set_border_side(&style_noborder, LV_BORDER_SIDE_NONE);
    lv_style_set_border_width(&style_noborder, 0);
    lv_style_set_outline_width(&style_noborder, 0);
   
    tileview = lv_tileview_create(lv_scr_act());
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(tileview, LV_DIR_ALL); 
    lv_obj_set_scroll_snap_x(tileview, LV_SCROLL_SNAP_CENTER); 
    lv_obj_set_size(tileview, LV_PCT(100), LV_PCT(100));

    /* Homescreen tile: provides an overview of the most important states */
    lv_obj_t * tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_VER);
    lv_obj_set_size(tile, LV_PCT(100), LV_PCT(100));
    create_dashboard(tile, nav_group);

    /* Menu list tile: a scrollable list for navigating to submenues, details, settings etc. */
    tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_VER);
    lv_obj_set_size(tile, LV_PCT(100), LV_PCT(100));
    create_list_menu(tile, nav_group);
    
    /* Create the task used to force refresh the UI */
    refr_task = lv_timer_create(wakeup_task, REFR_TIME, NULL);

    lvgl_run();
    return 0;
}
