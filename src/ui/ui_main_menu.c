#include "ui/ui_main_menu.h"

#include <stdio.h>

#include <log/log.h>
#include <lvgl/lvgl.h>

#include "common.hh"
#include "core/app_state.h"
#include "driver/hardware.h"
#include "driver/mcp3021.h"
#include "driver/oled.h"
#include "ui/page_autoscan.h"
#include "ui/page_clock.h"
#include "ui/page_common.h"
#include "ui/page_connections.h"
#include "ui/page_fans.h"
#include "ui/page_focus_chart.h"
#include "ui/page_headtracker.h"
#include "ui/page_imagesettings.h"
#include "ui/page_playback.h"
#include "ui/page_power.h"
#include "ui/page_record.h"
#include "ui/page_scannow.h"
#include "ui/page_source.h"
#include "ui/page_version.h"
#include "ui/ui_image_setting.h"
#include "ui/ui_porting.h"
#include "ui/ui_style.h"

LV_IMG_DECLARE(img_arrow);

progress_bar_t progress_bar;

static lv_obj_t *menu;
static lv_obj_t *root_page;

/**
 * Page order is enforced by definition.
 */
static page_pack_t *page_packs[] = {
    &pp_scannow,
    &pp_source,
    &pp_imagesettings,
    &pp_power,
    &pp_fans,
    &pp_record,
    &pp_autoscan,
    &pp_connections,
    &pp_headtracker,
    &pp_playback,
    &pp_version,
    &pp_focus_chart,
    &pp_clock,
};

#define PAGE_COUNT (sizeof(page_packs) / sizeof(page_packs[0]))

static page_pack_t *find_pp(lv_obj_t *page) {
    for (uint32_t i = 0; i < PAGE_COUNT; i++) {
        if (page_packs[i]->page == page) {
            return page_packs[i];
        }
    }
    return NULL;
}

static void clear_all_icon(void) {
    for (uint32_t i = 0; i < PAGE_COUNT; i++) {
        lv_img_set_src(page_packs[i]->icon, LV_SYMBOL_DUMMY);
    }
}

static void menu_event_handler(lv_event_t *e) {
    clear_all_icon();

    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (pp) {
        lv_img_set_src(pp->icon, &img_arrow);
    }
}

void submenu_enter(void) {
    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if (pp->p_arr.max) {
        // if we have selectable entries, select the first one
        pp->p_arr.cur = 0;
        set_select_item(&pp->p_arr, pp->p_arr.cur);
    }

    if (pp->enter) {
        // if your page as a enter event handler, call it
        pp->enter();
    }
}

void submenu_right_button(right_button_input_t click_type) {
    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if (pp->on_right_button) {
        // if your page has a right_button event handler, call it
        pp->on_right_button(click_type);
    }
}

void submenu_roller(uint8_t key) {
    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if (pp->on_roller) {
        // if your page as a roller event handler, call it
        pp->on_roller(key);
    }

    if (pp->p_arr.max) {
        // if we have selectable entries, move selection
        if (key == DIAL_KEY_UP) {
            if (pp->p_arr.cur < pp->p_arr.max - 1)
                pp->p_arr.cur++;
            else
                pp->p_arr.cur = 0;
        } else if (key == DIAL_KEY_DOWN) {
            if (pp->p_arr.cur > 0)
                pp->p_arr.cur--;
            else
                pp->p_arr.cur = pp->p_arr.max - 1;
        }
        set_select_item(&pp->p_arr, pp->p_arr.cur);
    }
}

void submenu_exit() {
    LOGI("submenu_exit");
    app_state_push(APP_STATE_MAINMENU);

    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if (pp->exit) {
        // if your page as a exit event handler, call it
        pp->exit();
    }

    if (pp->p_arr.max) {
        // if we have selectable icons, reset the selector
        pp->p_arr.cur = 0;
        set_select_item(&pp->p_arr, -1);
    }
}

void submenu_click(void) {
    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if (pp->on_click) {
        // if your page as a click event handler, call it
        pp->on_click(DIAL_KEY_CLICK, pp->p_arr.cur);
    }

    if (pp->p_arr.max) {
        // if we have selectable icons, check if we hit the back button
        if (pp->p_arr.cur == pp->p_arr.max - 1) {
            submenu_exit();
        }
    }
}

void menu_nav(uint8_t key) {
    static int8_t selected = 0;
    LOGI("menu_nav: key = %d,sel = %d", key, selected);
    if (key == DIAL_KEY_DOWN) {
        selected--;
        if (selected < 0)
            selected += PAGE_COUNT;
    } else if (key == DIAL_KEY_UP) {
        selected++;
        if (selected >= PAGE_COUNT)
            selected -= PAGE_COUNT;
    }
    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), selected), LV_EVENT_CLICKED, NULL);
}

static void menu_reinit(void) {
    LOGI("menu_reinit");

    page_pack_t *pp = find_pp(lv_menu_get_cur_main_page(menu));
    if (!pp) {
        return;
    }

    if ((pp == &pp_scannow)) {
        scan_reinit();
    }

    if (pp->p_arr.max) {
        // if we have selectable icons, reset the selector
        pp->p_arr.cur = 0;
        set_select_item(&pp->p_arr, -1);
    }
}

bool main_menu_is_shown(void) {
    return !lv_obj_has_flag(menu, LV_OBJ_FLAG_HIDDEN);
}

void main_menu_show(bool is_show) {
    if (is_show) {
        menu_reinit();
        lv_obj_clear_flag(menu, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);
    }
}

static void main_menu_create_entry(lv_obj_t *menu, lv_obj_t *section, page_pack_t *pp) {
    LOGD("creating main menu entry %s", pp->name);

    pp->page = pp->create(menu, &pp->p_arr);

    lv_obj_t *cont = lv_menu_cont_create(section);

    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, pp->name);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);

    pp->icon = lv_img_create(cont);
    lv_img_set_src(pp->icon, &img_arrow);

    lv_obj_set_style_text_font(cont, &lv_font_montserrat_26, 0);
    lv_menu_set_load_page_event(menu, cont, pp->page);
}

void main_menu_init(void) {
    menu = lv_menu_create(lv_scr_act());
    // lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(menu, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(menu, lv_color_make(32, 32, 32), 0);
    lv_obj_set_style_border_width(menu, 2, 0);
    lv_obj_set_style_border_color(menu, lv_color_make(255, 0, 0), 0);
    lv_obj_set_style_border_side(menu, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL) - 500, lv_disp_get_ver_res(NULL) - 96);
    lv_obj_set_pos(menu, 250, 96);

    root_page = lv_menu_page_create(menu, "aaa");

    lv_obj_t *section = lv_menu_section_create(root_page);
    lv_obj_clear_flag(section, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(menu, menu_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    for (uint32_t i = 0; i < PAGE_COUNT; i++) {
        main_menu_create_entry(menu, section, page_packs[i]);
    }

    lv_obj_add_style(section, &style_rootmenu, LV_PART_MAIN);
    lv_obj_set_size(section, 250, 975);
    lv_obj_set_pos(section, 0, 0);

    lv_obj_set_size(root_page, 250, 975);
    lv_obj_set_pos(root_page, 0, 0);
    lv_obj_set_style_border_width(root_page, 0, 0);
    lv_obj_set_style_radius(root_page, 0, 0);

    lv_menu_set_sidebar_page(menu, root_page);
    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(lv_menu_get_sidebar_header(menu), LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lv_menu_get_cur_sidebar_page(menu), LV_OBJ_FLAG_SCROLLABLE);

    progress_bar.bar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(progress_bar.bar, 320, 20);
    lv_obj_align(progress_bar.bar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(progress_bar.bar, LV_OBJ_FLAG_HIDDEN);
    progress_bar.start = 0;
    progress_bar.val = 0;
}

void progress_bar_update() {
    static uint8_t state = 0; // 0=idle, 1= in process

    switch (state) {
    case 0:
        if (progress_bar.start) { // to start the progress bar
            state = 1;
            lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(progress_bar.bar, LV_OBJ_FLAG_HIDDEN);
            progress_bar.val = 0;
            // LOGI("Progress bar start");
        }
        break;

    case 1:
        if (progress_bar.start == 0) { // to end end progress bar
            state = 0;
            lv_obj_clear_flag(menu, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(progress_bar.bar, LV_OBJ_FLAG_HIDDEN);
            progress_bar.val = 0;
            // LOGI("Progress bar end");
        }
        break;
    }

    if (state == 1) {
        if (progress_bar.val < 100)
            progress_bar.val += 4;
        lv_bar_set_value(progress_bar.bar, progress_bar.val, LV_ANIM_OFF);
        lv_timer_handler();
    }
}
