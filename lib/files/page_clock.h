#pragma once
#include <lvgl.h>
#include <cstdio>    // ← add this



extern lv_obj_t *g_time_label;
extern lv_obj_t *g_date_label;

void page_clock_create(lv_obj_t *parent);
void page_clock_update(uint8_t h, uint8_t m, uint8_t s,
                       uint8_t day, uint8_t month, uint16_t year, uint8_t dow);
