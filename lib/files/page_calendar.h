#pragma once
#include <lvgl.h>

extern lv_obj_t *g_calendar;
extern uint16_t  g_cal_year;
extern uint8_t   g_cal_month;
extern uint16_t  g_today_year;
extern uint8_t   g_today_month;
extern uint8_t   g_today_day;

void page_calendar_create(lv_obj_t *parent);
void page_calendar_set_today(uint16_t year, uint8_t month, uint8_t day);
void page_calendar_prev_month(void);
void page_calendar_next_month(void);
