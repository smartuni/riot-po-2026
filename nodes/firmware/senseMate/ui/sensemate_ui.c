#include <errno.h>
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl_riot.h"
#include "disp_dev.h"
#include "lvgl/src/widgets/lv_slider.h"
#include "lvgl/src/extra/widgets/list/lv_list.h"
#include "lvgl/src/extra/widgets/tileview/lv_tileview.h"
#include "lvgl/src/core/lv_theme.h"
#include "lvgl/src/extra/themes/mono/lv_theme_mono.h"
#include "periph/gpio.h"
#include "include/ui.h"

#define THUMBWHEEL_PIN_DOWN   GPIO_PIN(0, 4)
#define THUMBWHEEL_PIN_SELECT GPIO_PIN(0, 5)
#define THUMBWHEEL_PIN_UP     GPIO_PIN(0, 28)

/* Stack for the ui thread */
static char _ui_thread_stack[THREAD_STACKSIZE_UI];

/* Must be lower than LVGL_INACTIVITY_PERIOD_MS for autorefresh */
#define REFR_TIME           200

extern lv_font_t helvetica_light_12;
extern lv_font_t helvetica10;
extern const lv_img_dsc_t gate_icon_32x32;
extern const lv_img_dsc_t tasks_icon_25x32;
extern const lv_img_dsc_t person_icon_32x32;
extern const lv_img_dsc_t bluetooth_icon_7x11;

static lv_timer_t *refr_task;
static lv_style_t style_noborder;
static lv_indev_drv_t indev_drv;
static lv_group_t *nav_group;
static lv_indev_t *indev;
static lv_obj_t *tileview;

static lv_obj_t *submenu_tile;

struct ui_dyn_menu_ctx_t;

static lv_obj_t *badge_lbl_gates;
static lv_obj_t *badge_lbl_tasks;
static lv_obj_t *badge_lbl_persons;

typedef void (*ui_dyn_menu_enter_cb_t)(struct ui_dyn_menu_ctx_t *ctx);
typedef void (*ui_dyn_menu_leave_cb_t)(struct ui_dyn_menu_ctx_t *ctx);

typedef struct ui_dyn_menu_ctx_t {
    lv_obj_t *tileview;
    lv_obj_t *tile;
    lv_group_t *nav_group;
    ui_dyn_menu_enter_cb_t enter;
    ui_dyn_menu_leave_cb_t leave;
} ui_dyn_menu_ctx_t;

ui_dyn_menu_ctx_t main_menu_ctx;
ui_dyn_menu_ctx_t gate_menu_ctx;
ui_dyn_menu_ctx_t settings_menu_ctx;
ui_dyn_menu_ctx_t *current_menu_ctx = NULL;

static void wakeup_task(lv_timer_t *param)
{
    (void)param;
    /* Force a wakeup of lvgl when each task is called: this ensures an activity
       is triggered and wakes up lvgl during the next LVGL_INACTIVITY_PERIOD ms */
    lvgl_wakeup();
}

static void _encoder_with_keys_read(lv_indev_drv_t * drv, lv_indev_data_t*data){
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

static void _btn_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    (void)obj;
    if(code == LV_EVENT_CLICKED) {
        printf("CLICKED %d@%p\n", code, obj);
        void *usr_data = lv_event_get_user_data(e);
        if (usr_data) {
            if (current_menu_ctx && current_menu_ctx->leave) {
                current_menu_ctx->leave(current_menu_ctx);
            }
            ui_dyn_menu_ctx_t *ctx = (ui_dyn_menu_ctx_t*)usr_data;
            current_menu_ctx = ctx;
            if (ctx->enter) {
                ctx->enter(ctx);
            }
        }
    } else if(code == LV_EVENT_FOCUSED) {
        printf("FOCUSED: %s\n", lv_label_get_text(lv_obj_get_child(obj, 0)));
    }
}

static void _create_list_menu(lv_obj_t *parent, lv_group_t *grp)
{
    lv_obj_t *list1 = lv_list_create(parent);
    lv_obj_set_size(list1, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list1);
    lv_obj_add_style(list1, &style_noborder, 0);

    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Closeby Gates");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, &gate_menu_ctx);

    btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "All Gates");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, &gate_menu_ctx);

    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Jobs");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);
    
    btn = lv_list_add_btn(list1, LV_SYMBOL_WARNING, "Messages");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "Settings");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, &settings_menu_ctx);

    lv_obj_add_flag(list1, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_OFF);
}

static void _create_gate_list(lv_obj_t *parent, lv_group_t *grp)
{
    lv_obj_t *list1 = lv_list_create(parent);
    lv_obj_set_size(list1, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list1);
    lv_obj_add_style(list1, &style_noborder, 0);

    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "Gate 1");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "Gate 2");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "Gate 3");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_NEW_LINE, "back");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, &main_menu_ctx);

    lv_obj_add_flag(list1, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_OFF);
}


static lv_obj_t *_add_badged_icon(lv_obj_t *parent, const void *img_src, const char *badge_str) {
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

static lv_obj_t *_add_header_label(lv_obj_t *parent, const char *txt)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, txt);
    lv_obj_center(lbl);
    lv_obj_set_style_pad_all(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return lbl;
}

static void _create_dashboard(lv_obj_t *parent, lv_group_t *grp)
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
    
    lv_obj_t *lora_lbl = _add_header_label(header_cont, "LoRa");
    lv_obj_t *usb_lbl = _add_header_label(header_cont, LV_SYMBOL_USB);
    
    /* empty space between left and right part of the header/status bar */
    lv_obj_t * header_pad;
    header_pad = lv_obj_create(header_cont);
    lv_obj_set_height(header_pad, LV_PCT(100));
    lv_obj_set_flex_grow(header_pad, 1);
    lv_obj_add_style(header_pad, &style_noborder, 0); 
    lv_obj_set_style_pad_all(header_pad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* right part of the header symbols */
    lv_obj_t *alert_lbl = _add_header_label(header_cont, LV_SYMBOL_BELL);
    lv_obj_t *bat_lbl = _add_header_label(header_cont, LV_SYMBOL_BATTERY_FULL);

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
    
    badge_lbl_gates = _add_badged_icon(dash_cont, &gate_icon_32x32, "5");
    badge_lbl_tasks = _add_badged_icon(dash_cont, &tasks_icon_25x32, "3");
    badge_lbl_persons = _add_badged_icon(dash_cont, &person_icon_32x32, "2");
    
    //TODO: store label references for status/visibility updates
    (void)alert_lbl;
    (void)bat_lbl;
    (void)lora_lbl;
    (void)usb_lbl;
}

static void _gate_menu_dyn_enter(ui_dyn_menu_ctx_t *c)
{
    c->nav_group = lv_group_create();
    lv_group_set_wrap(c->nav_group, true);
    _create_gate_list(c->tile, c->nav_group);
    lv_obj_set_tile(c->tileview, c->tile, LV_ANIM_ON);
    lv_indev_set_group(indev, c->nav_group);
}

static void _clear_tile_dyn_leave(ui_dyn_menu_ctx_t *c)
{
    /* remove group from input device */
    lv_indev_set_group(indev, NULL);

    /* delete group */
    lv_group_del(c->nav_group);

    uint32_t child_cnt = lv_obj_get_child_cnt(c->tile);
    printf("leave! (delete %lu children)\n", child_cnt);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(c->tile, i);
        lv_obj_del(child);
    }
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    void *user_data = lv_event_get_user_data(e);
    if (user_data) {
        lv_obj_t *slider_label = (lv_obj_t*)user_data;
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
        lv_label_set_text(slider_label, buf);
        lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }
}

static void _settings_menu_dyn_enter(ui_dyn_menu_ctx_t *c)
{
    c->nav_group = lv_group_create();
    lv_group_set_wrap(c->nav_group, true);

    lv_obj_t *list1 = lv_list_create(c->tile);
    lv_obj_set_size(list1, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list1);
    lv_obj_add_style(list1, &style_noborder, 0);

    //TODO: move to create function so this can be used with dynamic and static loading
    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "Bluetooth");
    lv_group_add_obj(c->nav_group, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(list1);
    lv_obj_center(slider);
    lv_obj_set_size(slider, LV_PCT(80), LV_SIZE_CONTENT);
    lv_group_add_obj(c->nav_group, slider);

    /*Create a label below the slider*/
    lv_obj_t *slider_label = lv_label_create(list1);
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);

    btn = lv_list_add_btn(list1, LV_SYMBOL_NEW_LINE, "back");
    lv_group_add_obj(c->nav_group, btn);
    lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, &main_menu_ctx);

    lv_obj_add_flag(list1, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_tile(c->tileview, c->tile, LV_ANIM_ON);
    lv_indev_set_group(indev, c->nav_group);
}

static void _switch_to_contex_tile_enter_cb(ui_dyn_menu_ctx_t *c)
{
    (void)c;
    lv_obj_set_tile(c->tileview, c->tile, LV_ANIM_ON);
    lv_indev_set_group(indev, c->nav_group);
}

static void *_ui_thread(void *arg)
{
    (void)arg;
    gpio_init(THUMBWHEEL_PIN_DOWN, GPIO_IN_PU);
    gpio_init(THUMBWHEEL_PIN_SELECT, GPIO_IN_PU);
    gpio_init(THUMBWHEEL_PIN_UP, GPIO_IN_PU);
    
    /* create a new input device */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = _encoder_with_keys_read;
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
    _create_dashboard(tile, nav_group);

    /* Menu list tile: a scrollable list for navigating to submenues, details, settings etc. */
    tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_VER);
    lv_obj_set_size(tile, LV_PCT(100), LV_PCT(100));
    _create_list_menu(tile, nav_group);
    main_menu_ctx.tileview = tileview;
    main_menu_ctx.tile = tile;
    main_menu_ctx.nav_group = nav_group;
    main_menu_ctx.enter = _switch_to_contex_tile_enter_cb;

    /* Menu list tile: a scrollable list for navigating to submenues, details, settings etc. */
    submenu_tile = lv_tileview_add_tile(tileview, 1, 1, LV_DIR_HOR);
    lv_obj_set_size(tile, LV_PCT(100), LV_PCT(100));

    gate_menu_ctx.tileview = tileview;
    gate_menu_ctx.tile = submenu_tile;
    gate_menu_ctx.enter = _gate_menu_dyn_enter;
    gate_menu_ctx.leave = _clear_tile_dyn_leave;
    
    settings_menu_ctx.tileview = tileview;
    settings_menu_ctx.tile = submenu_tile;
    settings_menu_ctx.enter = _settings_menu_dyn_enter;
    settings_menu_ctx.leave = _clear_tile_dyn_leave;

    /* Create the task used to force refresh the UI */
    refr_task = lv_timer_create(wakeup_task, REFR_TIME, NULL);

    lvgl_run();

    /* will never be reached, lvgl_run() is blocking */
    return NULL;
}

void sensemate_ui_update(ui_data_t *data)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", data->visible_gate_cnt);
    lv_label_set_text(badge_lbl_gates, buf);

    lv_snprintf(buf, sizeof(buf), "%d", data->pending_jobs_cnt);
    lv_label_set_text(badge_lbl_tasks, buf);

    lv_snprintf(buf, sizeof(buf), "%d", data->visible_mate_cnt);
    lv_label_set_text(badge_lbl_persons, buf);
}

int sensemate_ui_init(void)
{
    /* create the reception thread] */
    kernel_pid_t ui_pid = thread_create(_ui_thread_stack, sizeof(_ui_thread_stack),
                                        THREAD_PRIORITY_MAIN - 1,
                                        THREAD_CREATE_STACKTEST, _ui_thread, NULL,
                                        "ui");
    if (-EINVAL == ui_pid) {
        puts("[ui]: failed to create ui thread.");
        return -1;
    }else{
        printf("[ui]: thread started successfully.\n");
    }
    return 0;
}
