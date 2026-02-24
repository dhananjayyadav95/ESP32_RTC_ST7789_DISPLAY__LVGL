#pragma once
#include <lvgl.h>

void page_stopwatch_create(lv_obj_t *parent);
void page_stopwatch_update(void);   // call every loop()
void page_stopwatch_toggle(void);   // ACTION btn: start / stop
void page_stopwatch_reset(void);    // PREV btn: reset
