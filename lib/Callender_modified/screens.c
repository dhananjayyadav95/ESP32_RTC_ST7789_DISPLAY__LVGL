#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>


// #define LV_FONT_MONTSERRAT_16 1

objects_t objects;

// Global calendar widget pointer for external button access
lv_obj_t *g_calendar = NULL;
lv_obj_t *g_time_label = NULL;
lv_obj_t *g_date_label = NULL;   // ← add this

// Real date from RTC (set before create_screens() is called)
uint16_t g_today_year  = 2025;
uint8_t  g_today_month = 1;
uint8_t  g_today_day   = 1;

// Currently displayed month/year (navigation may differ from today)
uint16_t g_cal_year  = 2025;
uint8_t  g_cal_month = 1;



// Called from main sketch after reading RTC
void calendar_set_today(uint16_t year, uint8_t month, uint8_t day) {
    g_today_year  = year;
    g_today_month = month;
    g_today_day   = day;
    g_cal_year    = year;
    g_cal_month   = month;
    if (g_calendar != NULL) {
        lv_calendar_set_today_date(g_calendar, year, month, day);
        lv_calendar_set_month_shown(g_calendar, year, month);
    }
}

// Previous month button
void calendar_prev_month(void) {
    if (g_calendar == NULL) return;
    if (g_cal_month == 1) { g_cal_month = 12; g_cal_year--; }
    else { g_cal_month--; }
    lv_calendar_set_month_shown(g_calendar, g_cal_year, g_cal_month);
}

// Next month button
void calendar_next_month(void) {
    if (g_calendar == NULL) return;
    if (g_cal_month == 12) { g_cal_month = 1; g_cal_year++; }
    else { g_cal_month++; }
    lv_calendar_set_month_shown(g_calendar, g_cal_year, g_cal_month);
}

// Jump back to today's month (3rd button)
void calendar_goto_today(void) {
    if (g_calendar == NULL) return;
    g_cal_year  = g_today_year;
    g_cal_month = g_today_month;
    lv_calendar_set_month_shown(g_calendar, g_cal_year, g_cal_month);
}

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

// void create_screen_main() {
//     lv_obj_t *obj = lv_obj_create(0);
//     objects.main = obj;
//     lv_obj_set_pos(obj, 0, 0);
//     lv_obj_set_size(obj, 240, 320);
//     {
//         lv_obj_t *parent_obj = obj;

//         // ── Real-time clock label ──────────────────────────
//         {
//             lv_obj_t *obj = lv_label_create(parent_obj);
//             lv_obj_set_pos(obj, 0, 0);          // 80 px from top
//             lv_obj_set_size(obj, 240, 80);
//             lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
//             lv_obj_set_style_text_color(obj, lv_color_white(), 0);
//             lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, 0);
//             lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);  // transparent bg
//             lv_label_set_text(obj, "00:00:00");
//             g_time_label = obj;                  // save global reference
//         }
//         {
//             lv_obj_t *obj = lv_calendar_create(parent_obj);
//             lv_obj_set_pos(obj, 0, 80);
//             lv_obj_set_size(obj, 240, 240);
//             lv_calendar_add_header_arrow(obj);
//             lv_calendar_set_today_date(obj, g_today_year, g_today_month, g_today_day);
//             lv_calendar_set_month_shown(obj, g_cal_year, g_cal_month);

//             // Save global reference so buttons can control the calendar
//             g_calendar = obj;
//         }

        
//     }
    
//     tick_screen_main();
// }


void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(obj, 0, 0);

    {
        lv_obj_t *parent_obj = obj;

        // ── Clock panel (top 80px) ────────────────────────────────
        {
            lv_obj_t *clock_panel = lv_obj_create(parent_obj);
            lv_obj_set_pos(clock_panel, 0, 0);
            lv_obj_set_size(clock_panel, 240, 80);
            lv_obj_set_style_bg_color(clock_panel, lv_color_hex(0x1A1A2E), 0); // dark navy
            lv_obj_set_style_border_width(clock_panel, 0, 0);
            lv_obj_set_style_pad_all(clock_panel, 0, 0);
            lv_obj_set_style_radius(clock_panel, 0, 0);
            lv_obj_clear_flag(clock_panel, LV_OBJ_FLAG_SCROLLABLE);

            // Time label  HH:MM:SS
            lv_obj_t *time_lbl = lv_label_create(clock_panel);
            lv_obj_set_width(time_lbl, 240);
            lv_obj_set_style_text_align(time_lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_32, 0);
            lv_obj_set_style_text_color(time_lbl, lv_color_hex(0xFFFFFF), 0);
            lv_obj_align(time_lbl, LV_ALIGN_CENTER, 0, -8);
            lv_label_set_text(time_lbl, "00:00:00");
            g_time_label = time_lbl;

            // Date label  Mon, 24 Feb 2026
            lv_obj_t *date_lbl = lv_label_create(clock_panel);
            lv_obj_set_width(date_lbl, 240);
            lv_obj_set_style_text_align(date_lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(date_lbl, &lv_font_montserrat_14, 0);
            lv_obj_set_style_text_color(date_lbl, lv_color_hex(0xAAAAAA), 0);
            lv_obj_align(date_lbl, LV_ALIGN_CENTER, 0, 20);
            lv_label_set_text(date_lbl, "Loading...");
            g_date_label = date_lbl;
        }

        // ── Calendar (y=80, remaining 240px) ─────────────────────
        {
            lv_obj_t *obj = lv_calendar_create(parent_obj);
            lv_obj_set_pos(obj, 0, 80);
            lv_obj_set_size(obj, 240, 240);
            lv_calendar_add_header_arrow(obj);
            lv_calendar_set_today_date(obj, g_today_year, g_today_month, g_today_day);
            lv_calendar_set_month_shown(obj, g_cal_year, g_cal_month);
            g_calendar = obj;
        }
    }

    tick_screen_main();
}

void tick_screen_main() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
}