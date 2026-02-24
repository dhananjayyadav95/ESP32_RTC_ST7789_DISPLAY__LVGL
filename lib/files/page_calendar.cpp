/**
 * page_calendar.cpp — Page 2: Monthly calendar
 *
 *  ┌─────────────────────┐
 *  │  PREV:<  ACTION:>   │  hint bar 24px
 *  ├─────────────────────┤
 *  │   << Feb 2026 >>    │
 *  │  Mo Tu We Th Fr Sa  │
 *  │        1  2  3  4   │
 *  │   5  6  …           │  calendar 272px
 *  ├─────────────────────┤
 *  │      ○ ● ○ ○        │  page dots
 *  └─────────────────────┘
 *
 *  PREV   → previous month
 *  ACTION → next month
 *  (no "goto today" btn — today is always highlighted automatically)
 */

#include "page_calendar.h"
#include "page_indicator.h"


lv_obj_t *g_calendar    = NULL;
uint16_t  g_cal_year    = 2026;
uint8_t   g_cal_month   = 2;
uint16_t  g_today_year  = 2026;
uint8_t   g_today_month = 2;
uint8_t   g_today_day   = 24;

void page_calendar_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0D0D1A), 0);

    // Hint bar
    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_set_pos(hdr, 0, 0);
    lv_obj_set_size(hdr, 240, 24);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(0x1A1A3A), 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_all(hdr, 0, 0);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hint = lv_label_create(hdr);
    lv_obj_set_width(hint, 240);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x6666AA), 0);
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(hint, "PREV: < month    ACTION: > month");

    // Calendar
    lv_obj_t *cal = lv_calendar_create(parent);
    lv_obj_set_pos(cal, 0, 24);
    lv_obj_set_size(cal, 240, 276);
    lv_calendar_add_header_arrow(cal);
    lv_calendar_set_today_date(cal, g_today_year, g_today_month, g_today_day);
    lv_calendar_set_month_shown(cal, g_cal_year, g_cal_month);
    g_calendar = cal;

  page_indicator_create(parent, 1);
}

void page_calendar_set_today(uint16_t year, uint8_t month, uint8_t day) {
    g_today_year  = year;
    g_today_month = month;
    g_today_day   = day;
    g_cal_year    = year;
    g_cal_month   = month;
    if (g_calendar) {
        lv_calendar_set_today_date(g_calendar, year, month, day);
        lv_calendar_set_month_shown(g_calendar, year, month);
    }
}

void page_calendar_prev_month(void) {
    if (!g_calendar) return;
    if (g_cal_month == 1) { g_cal_month = 12; g_cal_year--; }
    else g_cal_month--;
    lv_calendar_set_month_shown(g_calendar, g_cal_year, g_cal_month);
}

void page_calendar_next_month(void) {
    if (!g_calendar) return;
    if (g_cal_month == 12) { g_cal_month = 1; g_cal_year++; }
    else g_cal_month++;
    lv_calendar_set_month_shown(g_calendar, g_cal_year, g_cal_month);
}
