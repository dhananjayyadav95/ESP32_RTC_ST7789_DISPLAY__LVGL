/**
 * page_clock.cpp — Page 1: Full-screen live clock
 *
 *  ┌─────────────────────┐
 *  │                     │
 *  │      HH:MM:SS       │  montserrat 48, white
 *  │                     │
 *  │  Mon, 24 Feb 2026   │  montserrat 20, grey
 *  │                     │
 *  │   NEXT → next page  │  hint, dim
 *  │                     │
 *  │      ● ○ ○ ○        │  page dots
 *  └─────────────────────┘
 */

#include "page_clock.h"
#include "page_indicator.h"


lv_obj_t *g_time_label = NULL;
lv_obj_t *g_date_label = NULL;

void page_clock_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0D0D1A), 0);

    // Time
    g_time_label = lv_label_create(parent);
    lv_obj_set_width(g_time_label, 240);
    lv_obj_set_style_text_align(g_time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(g_time_label, LV_ALIGN_CENTER, 0, -30);
    lv_label_set_text(g_time_label, "00:00:00");

    // Date
    g_date_label = lv_label_create(parent);
    lv_obj_set_width(g_date_label, 240);
    lv_obj_set_style_text_align(g_date_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_date_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(g_date_label, lv_color_hex(0x7777AA), 0);
    lv_obj_align(g_date_label, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(g_date_label, "---");

    // Hint
    lv_obj_t *hint = lv_label_create(parent);
    lv_obj_set_width(hint, 240);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x333355), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_text(hint, "NEXT: next page");

    page_indicator_create(parent, 0);

   
}

void page_clock_update(uint8_t h, uint8_t m, uint8_t s,
                       uint8_t day, uint8_t month, uint16_t year, uint8_t dow) {
    if (!g_time_label) return;

    char buf[12];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    lv_label_set_text(g_time_label, buf);

    if (!g_date_label) return;
    const char *days[]   = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                             "Jul","Aug","Sep","Oct","Nov","Dec"};
    char dbuf[32];
    snprintf(dbuf, sizeof(dbuf), "%s, %02u %s %04u",
             days[dow % 7], day, months[(month-1) % 12], year);
    lv_label_set_text(g_date_label, dbuf);
}
