/**
 * page_manager.cpp
 */

#include "page_manager.h"
#include "page_clock.h"
#include "page_calendar.h"
#include "page_stopwatch.h"
#include "page_todo.h"
#include <lvgl.h>

int g_current_page = PAGE_CLOCK;
static lv_obj_t *pages[PAGE_COUNT] = {NULL, NULL, NULL, NULL};

void page_manager_init(void) {
    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i] = lv_obj_create(NULL);
        lv_obj_set_size(pages[i], 240, 320);
        lv_obj_set_style_pad_all(pages[i], 0, 0);
        lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    page_clock_create(pages[PAGE_CLOCK]);
    page_calendar_create(pages[PAGE_CALENDAR]);
    page_stopwatch_create(pages[PAGE_STOPWATCH]);
    page_todo_create(pages[PAGE_TODO]);

    lv_screen_load(pages[PAGE_CLOCK]);
    g_current_page = PAGE_CLOCK;
}

// NEXT button — always advances one page, wraps around
// void page_next(void) {
//     g_current_page = (g_current_page + 1) % PAGE_COUNT;
//     lv_screen_load_anim(pages[g_current_page],
//                         LV_SCR_LOAD_ANIM_FADE_IN, 400, 0, false);
// }

void page_next(void) {
    int from = g_current_page;
    g_current_page = (g_current_page + 1) % PAGE_COUNT;

    lv_screen_load_anim(pages[g_current_page],
                        LV_SCR_LOAD_ANIM_FADE_IN,
                        400,    // duration ms — long enough to feel smooth
                        0,      // no delay
                        false); // don't delete old screen

    // Apply ease-out curve so it decelerates like Android
    lv_anim_t *a = lv_anim_get(pages[g_current_page], NULL);
    if (a) lv_anim_set_path_cb(a, lv_anim_path_ease_out);
}

// PREV button — context action per page
void page_handle_prev_btn(void) {
    switch (g_current_page) {
        case PAGE_CLOCK:      /* no action */                 break;
        case PAGE_CALENDAR:   page_calendar_prev_month();     break;
        case PAGE_STOPWATCH:  page_stopwatch_reset();         break;
        case PAGE_TODO:       page_todo_scroll_up();          break;
    }
}

// ACTION button — context action per page
void page_handle_action_btn(void) {
    switch (g_current_page) {
        case PAGE_CLOCK:      /* no action */                 break;
        case PAGE_CALENDAR:   page_calendar_next_month();     break;
        case PAGE_STOPWATCH:  page_stopwatch_toggle();        break;
        case PAGE_TODO:       page_todo_mark_done();          break;
    }
}
