#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *main;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

// ── Button navigation & RTC API ────────────────────────────────────────────
// Call calendar_set_today() once after reading the RTC, before or after
// create_screens(). It highlights the real date and opens the correct month.
void calendar_set_today(uint16_t year, uint8_t month, uint8_t day);

void calendar_prev_month(void);   // "Previous" button  → go back one month
void calendar_next_month(void);   // "Next"     button  → go forward one month
void calendar_goto_today(void);   // "Today"    button  → jump back to today's month

// Exposed globals (read-only from outside)
extern lv_obj_t  *g_calendar;
extern uint16_t   g_today_year;
extern uint8_t    g_today_month;
extern uint8_t    g_today_day;

extern lv_obj_t *g_time_label;
extern lv_obj_t *g_date_label;   // ← add this


// ADD these two lines:
extern uint16_t   g_cal_year;
extern uint8_t    g_cal_month;

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/