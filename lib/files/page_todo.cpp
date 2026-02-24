/**
 * page_todo.cpp — Page 4: To-Do list
 *
 *  ┌─────────────────────┐
 *  │     TO-DO LIST      │
 *  ├─────────────────────┤
 *  │ ▶ [ ] Buy groceries │  ← selected row (white bg highlight)
 *  │   [ ] Morning walk  │
 *  │   [x] Read 30 mins  │  ← done (green)
 *  │   [ ] Drink water   │
 *  │   [ ] Review notes  │
 *  │   [ ] Sleep by 11pm │
 *  │   [ ] Plan tomorrow │
 *  │   [ ] Exercise      │
 *  ├─────────────────────┤
 *  │   3/8 done          │
 *  │ PREV:up  ACTION:done│
 *  │      ○ ○ ○ ●        │
 *  └─────────────────────┘
 *
 *  PREV   → move selection UP   (wraps to bottom)
 *  ACTION → toggle selected task done / undone
 *  NEXT   → go to next page (handled by page_manager)
 *
 *  Edit tasks[] array below to change your task list.
 */

#include "page_todo.h"
#include "page_indicator.h"


#define MAX_TASKS 8

static struct { const char *text; bool done; }
tasks[MAX_TASKS] = {
    { "Buy groceries",    false },
    { "Morning walk",     false },
    { "Read 30 mins",     false },
    { "Drink 8 glasses",  false },
    { "Review notes",     false },
    { "Sleep by 11pm",    false },
    { "Plan tomorrow",    false },
    { "Exercise",         false },
};

static int  sel        = 0;   // selected row index
static lv_obj_t *rows[MAX_TASKS] = {};
static lv_obj_t *status_lbl      = NULL;

static void todo_refresh(void) {
    int done = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].done) done++;
        if (!rows[i]) continue;

        // Build label text
        char buf[48];
        snprintf(buf, sizeof(buf), " %s %s  %s",
                 i == sel ? ">" : " ",
                 tasks[i].done ? "[x]" : "[ ]",
                 tasks[i].text);
        lv_label_set_text(rows[i], buf);

        // Colour coding
        if (i == sel) {
            lv_obj_set_style_text_color(rows[i], lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_color(rows[i],   lv_color_hex(0x2A2A5A), 0);
            lv_obj_set_style_bg_opa(rows[i], LV_OPA_COVER, 0);
        } else if (tasks[i].done) {
            lv_obj_set_style_text_color(rows[i], lv_color_hex(0x33AA55), 0);
            lv_obj_set_style_bg_opa(rows[i], LV_OPA_TRANSP, 0);
        } else {
            lv_obj_set_style_text_color(rows[i], lv_color_hex(0x888899), 0);
            lv_obj_set_style_bg_opa(rows[i], LV_OPA_TRANSP, 0);
        }
    }

    if (status_lbl) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%d / %d done", done, MAX_TASKS);
        lv_label_set_text(status_lbl, buf);
    }
}

void page_todo_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0D0D1A), 0);

    // Title
    lv_obj_t *title = lv_label_create(parent);
    lv_obj_set_width(title, 240);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x7777AA), 0);
    lv_obj_set_pos(title, 0, 8);
    lv_label_set_text(title, "TO-DO LIST");

    // Task rows
    const int Y0 = 34, ROW_H = 26;
    for (int i = 0; i < MAX_TASKS; i++) {
        lv_obj_t *r = lv_label_create(parent);
        lv_obj_set_pos(r, 0, Y0 + i * ROW_H);
        lv_obj_set_size(r, 240, ROW_H);
        lv_obj_set_style_text_font(r, &lv_font_montserrat_14, 0);
        lv_obj_set_style_pad_left(r, 4, 0);
        lv_obj_set_style_pad_top(r, 4, 0);
        lv_obj_set_style_radius(r, 4, 0);
        rows[i] = r;
    }

    // Status
    status_lbl = lv_label_create(parent);
    lv_obj_set_width(status_lbl, 240);
    lv_obj_set_style_text_align(status_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(status_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(0x44BB77), 0);
    lv_obj_align(status_lbl, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_text(status_lbl, "0 / 8 done");

    // Hint
    lv_obj_t *hint = lv_label_create(parent);
    lv_obj_set_width(hint, 240);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x333355), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_label_set_text(hint, "PREV: up    ACTION: done    NEXT: page");

    page_indicator_create(parent, 3);



    todo_refresh();
}

void page_todo_scroll_up(void) {
    sel = (sel - 1 + MAX_TASKS) % MAX_TASKS;   // wraps to bottom
    todo_refresh();
}

void page_todo_mark_done(void) {
    tasks[sel].done = !tasks[sel].done;
    // Auto-advance to next undone task
    int next = (sel + 1) % MAX_TASKS;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[next].done) { sel = next; break; }
        next = (next + 1) % MAX_TASKS;
    }
    todo_refresh();
}
