/**
 * page_stopwatch.cpp — Page 3: Stopwatch
 *
 *  ┌─────────────────────┐
 *  │      STOPWATCH      │
 *  │                     │
 *  │      00:00:00       │  big white (running=green, stopped=red)
 *  │        .000         │  milliseconds
 *  │                     │
 *  │   ● RUNNING         │  status
 *  │                     │
 *  │  PREV:RESET ACTION:START/STOP │
 *  │                     │
 *  │      ○ ○ ● ○        │
 *  └─────────────────────┘
 */

#include "page_stopwatch.h"
#include <Arduino.h>
#include "page_indicator.h"


static bool     sw_running  = false;
static uint32_t sw_start_ms = 0;
static uint32_t sw_elapsed  = 0;

static lv_obj_t *sw_time_lbl   = NULL;
static lv_obj_t *sw_ms_lbl     = NULL;
static lv_obj_t *sw_status_lbl = NULL;

static void sw_render(uint32_t ms_total) {
    if (!sw_time_lbl || !sw_ms_lbl) return;
    uint32_t ms   =  ms_total % 1000;
    uint32_t secs = (ms_total /    1000) % 60;
    uint32_t mins = (ms_total /   60000) % 60;
    uint32_t hrs  = (ms_total / 3600000);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hrs, mins, secs);
    lv_label_set_text(sw_time_lbl, buf);
    snprintf(buf, sizeof(buf), ".%03lu", ms);
    lv_label_set_text(sw_ms_lbl, buf);
}

void page_stopwatch_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0D0D1A), 0);

    // Title
    lv_obj_t *title = lv_label_create(parent);
    lv_obj_set_width(title, 240);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x7777AA), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
    lv_label_set_text(title, "STOPWATCH");

    // Big time
    sw_time_lbl = lv_label_create(parent);
    lv_obj_set_width(sw_time_lbl, 240);
    lv_obj_set_style_text_align(sw_time_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(sw_time_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(sw_time_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(sw_time_lbl, LV_ALIGN_CENTER, 0, -30);
    lv_label_set_text(sw_time_lbl, "00:00:00");

    // Milliseconds
    sw_ms_lbl = lv_label_create(parent);
    lv_obj_set_width(sw_ms_lbl, 240);
    lv_obj_set_style_text_align(sw_ms_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(sw_ms_lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(sw_ms_lbl, lv_color_hex(0x555588), 0);
    lv_obj_align(sw_ms_lbl, LV_ALIGN_CENTER, 0, 25);
    lv_label_set_text(sw_ms_lbl, ".000");

    // Status
    sw_status_lbl = lv_label_create(parent);
    lv_obj_set_width(sw_status_lbl, 240);
    lv_obj_set_style_text_align(sw_status_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(sw_status_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(sw_status_lbl, lv_color_hex(0x555577), 0);
    lv_obj_align(sw_status_lbl, LV_ALIGN_CENTER, 0, 72);
    lv_label_set_text(sw_status_lbl, "READY");

    // Hint
    lv_obj_t *hint = lv_label_create(parent);
    lv_obj_set_width(hint, 240);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x333355), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_text(hint, "ACTION: start/stop    PREV: reset");

   page_indicator_create(parent, 2);  // change number per page

}

void page_stopwatch_update(void) {
    if (!sw_running) return;
    sw_render(sw_elapsed + (millis() - sw_start_ms));
}

void page_stopwatch_toggle(void) {
    if (sw_running) {
        // STOP
        sw_elapsed += millis() - sw_start_ms;
        sw_running  = false;
        sw_render(sw_elapsed);
        if (sw_time_lbl)
            lv_obj_set_style_text_color(sw_time_lbl, lv_color_hex(0xFF5544), 0);
        if (sw_status_lbl)
            lv_label_set_text(sw_status_lbl, "STOPPED");
    } else {
        // START
        sw_start_ms = millis();
        sw_running  = true;
        if (sw_time_lbl)
            lv_obj_set_style_text_color(sw_time_lbl, lv_color_hex(0x44FF88), 0);
        if (sw_status_lbl)
            lv_label_set_text(sw_status_lbl, "RUNNING");
    }
}

void page_stopwatch_reset(void) {
    sw_running = false;
    sw_elapsed = 0;
    sw_render(0);
    if (sw_time_lbl)
        lv_obj_set_style_text_color(sw_time_lbl, lv_color_hex(0xFFFFFF), 0);
    if (sw_status_lbl)
        lv_label_set_text(sw_status_lbl, "READY");
}
