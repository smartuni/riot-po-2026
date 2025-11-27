#include <errno.h>
#include <stdio.h>
#include "od.h"
#include "event.h"
#include "lvgl/lvgl.h"
#include "lvgl_riot.h"
#include "disp_dev.h"
#include "lvgl/src/widgets/lv_slider.h"
#include "lvgl/src/extra/widgets/list/lv_list.h"
#include "lvgl/src/extra/widgets/tileview/lv_tileview.h"
#include "lvgl/src/core/lv_theme.h"
#include "lvgl/src/extra/themes/mono/lv_theme_mono.h"
#include "periph/gpio.h"
#include "include/sensemate_ui.h"
#include "timex.h"
#define LOG_LEVEL   LOG_NONE
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[ui]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[ui]: " __VA_ARGS__)

#ifndef CONFIG_LVGL_INACTIVITY_PERIOD_MS
#define CONFIG_LVGL_INACTIVITY_PERIOD_MS    (5 * MS_PER_SEC)    /* 5s */
#endif

#ifndef CONFIG_LVGL_TASK_HANDLER_DELAY_MS
#define CONFIG_LVGL_TASK_HANDLER_DELAY_MS   (5)                 /* 5ms */
#endif

#ifndef LVGL_THREAD_FLAG
#define LVGL_THREAD_FLAG                    (1 << 7)
#endif

#define XSTR(s) #s
#define STR(s) XSTR(s)

#define THUMBWHEEL_PIN_DOWN   GPIO_PIN(0, 4)
#define THUMBWHEEL_PIN_SELECT GPIO_PIN(0, 5)
#define THUMBWHEEL_PIN_UP     GPIO_PIN(0, 28)
#define INVALID_GATE_MARKER (0xFFFFFFFF)
/* Stack for the ui thread */
static char _ui_thread_stack[THREAD_STACKSIZE_UI];

static void _ui_trigger_refresh(event_t *event);
static void _custom_lvgl_wakeup(void);
event_t event_trigger_ui_refresh = { .handler = _ui_trigger_refresh };

/* Must be lower than LVGL_INACTIVITY_PERIOD_MS for autorefresh */
#define REFR_TIME           200

static ui_data_t _ui_state;
static const ui_data_cbs_t *_data_cbs;

extern lv_font_t helvetica_light_12;
extern lv_font_t helvetica10;
extern lv_font_t font_goldfish;
extern const lv_font_t lv_font_unscii_8;
extern const lv_img_dsc_t lock_open_icon_16x16;
extern const lv_img_dsc_t lock_closed_icon_16x16;
extern const lv_img_dsc_t gate_icon_open_8x9;
extern const lv_img_dsc_t gate_icon_closed_8x9;
extern const lv_img_dsc_t gate_icon_32x32;
extern const lv_img_dsc_t tasks_icon_25x32;
extern const lv_img_dsc_t person_icon_32x32;
extern const lv_img_dsc_t bluetooth_icon_7x11;
extern const lv_img_dsc_t arrow_up_icon_5x9;
extern const lv_img_dsc_t arrow_down_icon_5x9;
extern const lv_img_dsc_t arrow_up_down_icon_5x9;

static lv_timer_t *refr_task;
static lv_style_t style_noborder;
static lv_style_t style_focused_button;
static lv_indev_drv_t indev_drv;
static lv_group_t *nav_group;
static lv_indev_t *indev;
static lv_obj_t *tileview;

static lv_obj_t *submenu_tile;

struct ui_dyn_menu_ctx_t;

static lv_obj_t *badge_lbl_gates;
static lv_obj_t *badge_lbl_tasks;
static lv_obj_t *badge_lbl_persons;

static lv_obj_t *alert_lbl;
static lv_obj_t *bat_lbl;

typedef void (*ui_dyn_menu_enter_cb_t)(struct ui_dyn_menu_ctx_t *ctx);
typedef void (*ui_dyn_menu_leave_cb_t)(struct ui_dyn_menu_ctx_t *ctx);
typedef void (*ui_update_cb_t)(struct ui_dyn_menu_ctx_t *ctx);

typedef struct ui_dyn_menu_ctx_t {
    lv_obj_t *tileview;
    lv_obj_t *tile;
    lv_group_t *nav_group;
    ui_dyn_menu_enter_cb_t enter;
    ui_dyn_menu_leave_cb_t leave;
    ui_update_cb_t update;
} ui_dyn_menu_ctx_t;

typedef struct {
    node_id_t gate_id;
    lv_obj_t *delete_on_exit;
    lv_group_t *prev_nav_group;
} ui_gate_edit_ctx_t;

/* In the UI the selected gate should only ever by a single one,
 * so a global stzate is sufficient. */
ui_gate_edit_ctx_t _gate_edit_ctx;

ui_dyn_menu_ctx_t main_menu_ctx;
ui_dyn_menu_ctx_t gate_menu_all_ctx;
ui_dyn_menu_ctx_t gate_menu_closeby_ctx;
ui_dyn_menu_ctx_t settings_menu_ctx;
ui_dyn_menu_ctx_t *current_menu_ctx = NULL;

/* how long to show/hide a "connectivity state" ui element
 * which blinks while establishing the connection. */
#define CONNECTING_BLINK_TIME_MS        500
//static lv_timer_t *_lora_conn_state_blink_timer = NULL;

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *rx_tx_icon_obj;
    ui_connection_state_t *conn_state;
    lv_timer_t *blink_timer;
} ui_conn_state_obj_ctx_t;

static ui_conn_state_obj_ctx_t _ble_conn_state_ctx = {
    .obj = NULL,
    .conn_state = &_ui_state.ble_state,
    .blink_timer = NULL,
};

static ui_conn_state_obj_ctx_t _lora_conn_state_ctx = {
    .obj = NULL,
    .conn_state = &_ui_state.lora_state,
    .blink_timer = NULL,
};

static ui_conn_state_obj_ctx_t _usb_conn_state_ctx = {
    .obj = NULL,
    .conn_state = &_ui_state.usb_state,
    .blink_timer = NULL,
};

static void wakeup_task(lv_timer_t *param)
{
    (void)param;
    /* Force a wakeup of lvgl when each task is called: this ensures an activity
       is triggered and wakes up lvgl during the next LVGL_INACTIVITY_PERIOD ms */
    _custom_lvgl_wakeup();
}

static void establ_conn_blink_task(lv_timer_t *timer)
{
    ui_conn_state_obj_ctx_t *ctx = timer->user_data;
    if (*ctx->conn_state == ESTABLISHING_CONNECTION) {
        if (lv_obj_has_flag(ctx->obj, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(ctx->obj, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ctx->obj, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
       lv_timer_del(timer);
       ctx->blink_timer = NULL;
    }
}

static void rx_tx_hide_task(lv_timer_t *timer)
{
    ui_conn_state_obj_ctx_t *ctx = timer->user_data;
    lv_obj_add_flag(ctx->rx_tx_icon_obj, LV_OBJ_FLAG_HIDDEN);
    lv_timer_del(timer);
    ctx->blink_timer = NULL;
    /* reset RX or TX state to connected */
    *ctx->conn_state = CONNECTED;
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

static void _menu_btn_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    (void)obj;
    if(code == LV_EVENT_CLICKED) {
        //printf("CLICKED %d@%p\n", code, obj);
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
    }
}

const char * _observed_gate_state_opts[] = {"Open", "Closed", "Cancel", NULL};

static void _gate_state_prompt_btn_handler(lv_event_t * e)
{
    void *usr_data = lv_event_get_user_data(e);
    int btn_idx = (int)usr_data;
    if (btn_idx != 2) { /* cancel*/
        gate_state_t observed_state = (btn_idx == 0) ? GATE_STATE_OPEN : GATE_STATE_CLOSED;
        ui_data_element_t data_elem = {
            .data.gate_observation = {
                            .state = observed_state,
            },
        };
        //TODO change the gate id type to something that can be assigned by value directly
        //     or introduce some better accessor/translator functions.
        memcpy(data_elem.data.gate_observation.gate_id, _gate_edit_ctx.gate_id, sizeof(node_id_t));
        _data_cbs->put_gate_observation(&data_elem);
    }

    lv_indev_set_group(indev, _gate_edit_ctx.prev_nav_group);
    /* close the dialog this button belongs to */
    lv_obj_del(_gate_edit_ctx.delete_on_exit);
}

static lv_obj_t* _create_dialog(lv_obj_t *parent, const char *txt, const char *options[])
{
    lv_obj_t *dialog_cont = lv_obj_create(parent);
    lv_obj_set_size(dialog_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(dialog_cont, &style_noborder, 0);
    lv_obj_center(dialog_cont);
    lv_obj_add_flag(dialog_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_t *label = lv_label_create(dialog_cont);
    lv_label_set_text(label, txt);
    lv_obj_align_to(label, dialog_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 1);

    int elem_cnt = 0;
    while (options[elem_cnt++] != 0) {};
    elem_cnt--;

    lv_obj_t *cont_row = lv_obj_create(dialog_cont);
    lv_obj_set_size(cont_row, LV_PCT(50 * elem_cnt), LV_PCT(50));
    lv_obj_align(cont_row, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(cont_row, &style_noborder, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_SPACE_BETWEEN);

    lv_obj_set_scroll_snap_x(cont_row, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(dialog_cont, LV_SCROLLBAR_MODE_OFF);

    lv_group_t *nav_group = lv_group_create();
    lv_group_set_wrap(nav_group, true);
    lv_indev_set_group(indev, nav_group);

    lv_obj_t *first_btn = NULL;
    for (int i = 0; options[i] != NULL; i++) {
        lv_obj_t *btn = lv_btn_create(cont_row);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, options[i]);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(label, 1);
        lv_obj_add_event_cb(btn, _gate_state_prompt_btn_handler, LV_EVENT_CLICKED, (void*)i);

        lv_obj_add_style(btn, &style_focused_button, LV_STATE_FOCUSED);
        lv_group_add_obj(nav_group, btn);
        if (i == 0) {
            first_btn = btn;
        }
    }

    /* focus must be set after all the buttons were added to render correctly */
    if (first_btn) {
        lv_group_focus_obj(first_btn);
    }
    return dialog_cont;
}

void _uint32_to_node_id(uint32_t i, node_id_t ni)
{
    memcpy(ni, &i, sizeof(node_id_t));
}

void _node_id_to_uint32(node_id_t ni, uint32_t *i)
{
    memcpy(i, ni, sizeof(node_id_t));
}

static void _gate_edit_btn_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    (void)obj;
    if(code == LV_EVENT_CLICKED) {
        void *usr_data = lv_event_get_user_data(e);
        if (usr_data) {
            node_id_t gate_id;
            _uint32_to_node_id((uint32_t)usr_data, gate_id);

            /* TODO: is there a safer /more generic  way for this? */
            lv_obj_t *tile = lv_obj_get_parent(lv_obj_get_parent(obj));
            char prompt[32];
            //TODO: replace this hack with a proper translation/ lookup function
            lv_snprintf(prompt, sizeof(prompt), "Report Gate-%d as:", gate_id[3]);
            lv_obj_t *dialog = _create_dialog(tile, prompt, _observed_gate_state_opts);
            memcpy(_gate_edit_ctx.gate_id, gate_id, sizeof(node_id_t));
            _gate_edit_ctx.prev_nav_group = lv_obj_get_group(obj);
            _gate_edit_ctx.delete_on_exit = dialog;
        }
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
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, &gate_menu_closeby_ctx);

    btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, "All Gates");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, &gate_menu_all_ctx);

    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Jobs");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, NULL);
    
    btn = lv_list_add_btn(list1, LV_SYMBOL_WARNING, "Messages");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_btn(list1, LV_SYMBOL_SETTINGS, "Settings");
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, &settings_menu_ctx);

    lv_obj_add_flag(list1, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_OFF);
}

static void _create_gate_list(lv_obj_t *parent, lv_group_t *grp, bool only_closeby)
{
    lv_obj_t *list1 = lv_list_create(parent);
    lv_obj_set_size(list1, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list1);
    lv_obj_add_style(list1, &style_noborder, 0);
    ui_data_element_iter_cb_t iter_cb = _data_cbs->all_gates_iter;

    ui_data_element_t gate_elem = { .iter_ctx.ptr = NULL };
    while(iter_cb(&gate_elem)) {
        gate_local_info_entry_t *li = &gate_elem.data.local_gate_info;
        if (only_closeby && li->beacon_rssi < -70) {
            continue;
        }
        char buf[16];
        //TODO: replace this hack with a proper translation/ lookup function
        lv_snprintf(buf, sizeof(buf), "Gate %d", ((uint8_t*)li->gateID)[3]);
        lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_LIST, buf);

        uint32_t id;
        _node_id_to_uint32(li->gateID, &id);

        lv_obj_set_user_data(btn, (void*)id);
        if (li->sensor_data_present) {
            lv_obj_t *icon = lv_img_create(btn);
            if (li->sensor_state == GATE_STATE_OPEN) {
                lv_img_set_src(icon, &lock_open_icon_16x16);
            } else {
                lv_img_set_src(icon, &lock_closed_icon_16x16);
            }
        }
        lv_group_add_obj(grp, btn);

        lv_obj_add_event_cb(btn, _gate_edit_btn_handler, LV_EVENT_CLICKED, (void*)id);
    }

    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_NEW_LINE, "back");
    lv_obj_set_user_data(btn, (void*)INVALID_GATE_MARKER);
    lv_group_add_obj(grp, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, &main_menu_ctx);

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

    /* hide by default, should be updated dynamically on value changes */
    // TODO: this should be done during setup but currently causes a layout artifact
    //       where the bottom right bart of the badge is cut of once visible again
    //lv_obj_add_flag(badge_lbl, LV_OBJ_FLAG_HIDDEN);
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

static void _update_badge_counter_label(lv_obj_t *lbl, int value)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", value);
    lv_label_set_text(lbl, buf);
    if (!value) {
        lv_obj_add_flag(lbl, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(lbl, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t* _add_rx_tx_icon_obj(lv_obj_t *parent)
{
    lv_obj_t *icon = lv_img_create(parent);
    lv_img_set_src(icon, &arrow_up_down_icon_5x9);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
    return icon;
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
    _ble_conn_state_ctx.obj = icon;
    _ble_conn_state_ctx.rx_tx_icon_obj = _add_rx_tx_icon_obj(header_cont);

    _lora_conn_state_ctx.obj = _add_header_label(header_cont, "LoRa");
    lv_obj_set_style_text_font(_lora_conn_state_ctx.obj, &font_goldfish, 0);
    _lora_conn_state_ctx.rx_tx_icon_obj = _add_rx_tx_icon_obj(header_cont);
    
    _usb_conn_state_ctx.obj = _add_header_label(header_cont, LV_SYMBOL_USB);
    _usb_conn_state_ctx.rx_tx_icon_obj = _add_rx_tx_icon_obj(header_cont);

    /* empty space between left and right part of the header/status bar */
    lv_obj_t * header_pad;
    header_pad = lv_obj_create(header_cont);
    lv_obj_set_height(header_pad, LV_PCT(100));
    lv_obj_set_flex_grow(header_pad, 1);
    lv_obj_add_style(header_pad, &style_noborder, 0); 
    lv_obj_set_style_pad_all(header_pad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* right part of the header symbols */
    lv_obj_t *dev_id_lbl = _add_header_label(header_cont, "Mate-" STR(RIOT_CONFIG_DEVICE_ID));
    lv_obj_set_style_text_font(dev_id_lbl, &font_goldfish, 0);

    //alert_lbl = _add_header_label(header_cont, LV_SYMBOL_BELL);
    (void)alert_lbl;
    bat_lbl = _add_header_label(header_cont, LV_SYMBOL_BATTERY_FULL);

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
    
    badge_lbl_gates = _add_badged_icon(dash_cont, &gate_icon_32x32, "0");
    badge_lbl_tasks = _add_badged_icon(dash_cont, &tasks_icon_25x32, "0");
    badge_lbl_persons = _add_badged_icon(dash_cont, &person_icon_32x32, "0");
}

lv_obj_t * _get_gate_state_ui_handle_from_list(lv_obj_t *list, node_id_t gateid)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(list, i);
        if(lv_obj_check_type(child, &lv_list_btn_class)) {
            //const char *btn_str = lv_list_get_btn_text(l, child);
            uint32_t user_data = (uint32_t)lv_obj_get_user_data(child);
            if (user_data != INVALID_GATE_MARKER) {
                //gate_id_t gi = (gate_id_t)(uint32_t)lv_obj_get_user_data(child);
                node_id_t gi;
                _uint32_to_node_id(user_data, gi);
                //
                //printf("%s -> gate id = %u\n", btn_str, gi);
                //if (gi == gateid) {
                if (memcmp(gi, gateid, sizeof(node_id_t)) == 0) {
                    return child;
                }
            }
        }
    }
    return NULL;
}

lv_obj_t * _get_state_img_obj_from_gate_ui_handle(lv_obj_t *guih)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(guih);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(guih, i);
        if(lv_obj_check_type(child, &lv_img_class)) {
            const void *img_src = lv_img_get_src(child);
            if (img_src == &lock_open_icon_16x16 ||
                img_src == &lock_closed_icon_16x16) {
                return child;
            }
        }
    }
    return NULL;
}

// This method is hardcoded for the gate list object tree!
// this must be adjusted if the gate list is modified
void _ui_update_gate_list(struct ui_dyn_menu_ctx_t *ctx)
{
    lv_obj_t *t = ctx->tile;
    lv_obj_t *l = lv_obj_get_child(t, 0);

    /* we expect a list that holds buttons */
    if(!lv_obj_check_type(l, &lv_list_class)) {
        return;
    }

    ui_data_element_iter_cb_t iter_cb = _data_cbs->all_gates_iter;
    ui_data_element_t gate_elem = { .iter_ctx.ptr = NULL };

    while(iter_cb(&gate_elem)) {
        gate_local_info_entry_t *li = &gate_elem.data.local_gate_info;
        if (li->sensor_data_present) {
            lv_obj_t * btn = _get_gate_state_ui_handle_from_list(l, li->gateID);
            if (btn) {
                lv_obj_t *img = _get_state_img_obj_from_gate_ui_handle(btn);
                if (img) {
                    if (li->sensor_state == GATE_STATE_OPEN) {
                        lv_img_set_src(img, &lock_open_icon_16x16);
                    } else {
                        lv_img_set_src(img, &lock_closed_icon_16x16);
                    }
                }
            }
        }
    }
}

static void _gate_menu_dyn_enter_mode(ui_dyn_menu_ctx_t *c, bool only_closeby)
{
    c->nav_group = lv_group_create();
    lv_group_set_wrap(c->nav_group, true);
    _create_gate_list(c->tile, c->nav_group, only_closeby);
    lv_obj_set_tile(c->tileview, c->tile, LV_ANIM_ON);

    c->update = _ui_update_gate_list;
    lv_indev_set_group(indev, c->nav_group);
}

static void _gate_menu_dyn_enter_all(ui_dyn_menu_ctx_t *c)
{
    _gate_menu_dyn_enter_mode(c, false);
}

static void _gate_menu_dyn_enter_closeby(ui_dyn_menu_ctx_t *c)
{
    _gate_menu_dyn_enter_mode(c, true);
}

static void _clear_tile_dyn_leave(ui_dyn_menu_ctx_t *c)
{
    c->update = NULL;

    /* remove group from input device */
    lv_indev_set_group(indev, NULL);

    /* delete group */
    lv_group_del(c->nav_group);

    uint32_t child_cnt = lv_obj_get_child_cnt(c->tile);
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
    //lv_obj_add_event_cb(btn, _btn_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * slider = lv_slider_create(list1);
    lv_obj_set_size(slider, LV_PCT(80), LV_SIZE_CONTENT);
    //TODO: this does not center the element in the list as intended
    lv_obj_center(slider);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_group_add_obj(c->nav_group, slider);

    /*Create a label below the slider*/
    lv_obj_t *slider_label = lv_label_create(list1);
    lv_label_set_text(slider_label, "0%");
    lv_obj_align(slider_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);

    btn = lv_list_add_btn(list1, LV_SYMBOL_NEW_LINE, "back");
    lv_group_add_obj(c->nav_group, btn);
    lv_obj_add_event_cb(btn, _menu_btn_handler, LV_EVENT_CLICKED, &main_menu_ctx);

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

static kernel_pid_t _task_thread_pid;
static bool _refresh_ui_elements_needed = false;

static void _custom_lvgl_run(void)
{
    _task_thread_pid = thread_getpid();
    while (1) {
        /* Normal operation (no sleep) in < CONFIG_LVGL_INACTIVITY_PERIOD_MS msec
           inactivity */
        if (lv_disp_get_inactive_time(NULL) < CONFIG_LVGL_INACTIVITY_PERIOD_MS) {
            if (_refresh_ui_elements_needed) {
                /* consume refresh flag */
                _refresh_ui_elements_needed = false;
                /* update ui elements such as labels etc. */
                sensemate_ui_update();
            }
            lv_timer_handler();
        } else {
            /* Block after LVGL_ACTIVITY_PERIOD msec inactivity */
            thread_flags_wait_one(LVGL_THREAD_FLAG);
            /* trigger an activity so the task handler is called on the next loop */
            lv_disp_trig_activity(NULL);
        }

        ztimer_sleep(ZTIMER_MSEC, CONFIG_LVGL_TASK_HANDLER_DELAY_MS);
    }
}

static void _custom_lvgl_wakeup(void)
{
    thread_t *tcb = thread_get(_task_thread_pid);
    thread_flags_set(tcb, LVGL_THREAD_FLAG);
}

static void _ui_trigger_refresh(event_t *event)
{
    (void)event;
    _LOGDBG("trigger refresh...\n");
    _refresh_ui_elements_needed = true;
    _custom_lvgl_wakeup();
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

    lv_style_init(&style_focused_button);
    lv_style_set_bg_color(&style_focused_button, lv_color_white());
    lv_style_set_text_color(&style_focused_button, lv_color_black());

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

    gate_menu_all_ctx.tileview = tileview;
    gate_menu_all_ctx.tile = submenu_tile;
    gate_menu_all_ctx.enter = _gate_menu_dyn_enter_all;
    gate_menu_all_ctx.leave = _clear_tile_dyn_leave;

    gate_menu_closeby_ctx.tileview = tileview;
    gate_menu_closeby_ctx.tile = submenu_tile;
    gate_menu_closeby_ctx.enter = _gate_menu_dyn_enter_closeby;
    gate_menu_closeby_ctx.leave = _clear_tile_dyn_leave;

    settings_menu_ctx.tileview = tileview;
    settings_menu_ctx.tile = submenu_tile;
    settings_menu_ctx.enter = _settings_menu_dyn_enter;
    settings_menu_ctx.leave = _clear_tile_dyn_leave;

    /* Create the task used to force refresh the UI */
    refr_task = lv_timer_create(wakeup_task, REFR_TIME, NULL);

    sensemate_ui_update();
    _custom_lvgl_run();

    /* will never be reached, lvgl_run() is blocking */
    return NULL;
}

int sensemate_ui_init(ui_data_cbs_t *data_cbs)
{
    _data_cbs = data_cbs;
    _ui_state.visible_gate_cnt = 0;
    _ui_state.pending_jobs_cnt = 0;
    _ui_state.visible_mate_cnt = 0;
    _ui_state.lora_state = DISCONNECTED;
    _ui_state.ble_state = DISCONNECTED;
    _ui_state.usb_state = DISCONNECTED;

    /* create the reception thread] */
    kernel_pid_t ui_pid = thread_create(_ui_thread_stack, sizeof(_ui_thread_stack),
                                        THREAD_PRIORITY_MAIN - 2,
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

ui_data_t *sensemate_ui_get_state(void)
{
    return &_ui_state;
}

static void _update_connection_state_obj(ui_conn_state_obj_ctx_t *ctx)
{
    ui_connection_state_t conn_state = *ctx->conn_state;
    lv_obj_t *obj = ctx->obj;

    if (conn_state == CONNECTED) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if (conn_state == DISCONNECTED){
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if (conn_state == ESTABLISHING_CONNECTION){
        /* only if not set up already */
        if (!ctx->blink_timer) {
            /* setup a timer which toggles the visibility of the lora label object.
             * The timer is automatically deleted after the linked state is set
             * to CONNECTED */
            ctx->blink_timer = lv_timer_create(establ_conn_blink_task,
                                               CONNECTING_BLINK_TIME_MS,
                                               ctx);
        }
    } else if (conn_state == RECEIVED || conn_state == TRANSMITTED){
        if (!ctx->blink_timer) {
            if (conn_state == RECEIVED) {
                lv_img_set_src(ctx->rx_tx_icon_obj, &arrow_down_icon_5x9);
            } else {
                lv_img_set_src(ctx->rx_tx_icon_obj, &arrow_up_icon_5x9);
            }

            lv_obj_clear_flag(ctx->rx_tx_icon_obj, LV_OBJ_FLAG_HIDDEN);

            ctx->blink_timer = lv_timer_create(rx_tx_hide_task,
                                               CONNECTING_BLINK_TIME_MS,
                                               ctx);
        }

    }
}
void sensemate_ui_update(void)
{
    _LOGDBG("%s\n", __func__);
    _update_badge_counter_label(badge_lbl_gates, _ui_state.visible_gate_cnt);
    _update_badge_counter_label(badge_lbl_tasks, _ui_state.pending_jobs_cnt);
    _update_badge_counter_label(badge_lbl_persons, _ui_state.visible_mate_cnt);

    _update_connection_state_obj(&_lora_conn_state_ctx);
    _update_connection_state_obj(&_ble_conn_state_ctx);
    _update_connection_state_obj(&_usb_conn_state_ctx);

    //TODO: add some method to invalidate the gate list in case it is currently shown
    if(current_menu_ctx) {
        if (current_menu_ctx->update) {
            current_menu_ctx->update(current_menu_ctx);
        }
    }
}
