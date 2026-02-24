#pragma once
#include <lvgl.h>
#include <cstdio>    // ← add this


void page_todo_create(lv_obj_t *parent);
void page_todo_scroll_up(void);    // PREV btn
void page_todo_mark_done(void);    // ACTION btn: toggle selected task
