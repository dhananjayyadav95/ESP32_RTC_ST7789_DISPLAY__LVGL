#include "page_indicator.h"

#define DOT_COUNT   4
#define DOT_W_SMALL 6
#define DOT_W_LARGE 18   // active dot stretches like Android pill
#define DOT_H       6
#define DOT_GAP     10
#define DOT_Y       -10  // offset from bottom

static lv_obj_t *dots[DOT_COUNT] = {};

void page_indicator_create(lv_obj_t *parent, int active_index) {
    // Total width: 3 small dots + 1 large + gaps
    int total_w = DOT_W_LARGE + (DOT_COUNT - 1) * DOT_W_SMALL
                  + (DOT_COUNT - 1) * DOT_GAP;
    int x_start = (240 - total_w) / 2;
    int x = x_start;

    for (int i = 0; i < DOT_COUNT; i++) {
        lv_obj_t *d = lv_obj_create(parent);
        int w = (i == active_index) ? DOT_W_LARGE : DOT_W_SMALL;
        lv_obj_set_size(d, w, DOT_H);
        lv_obj_set_pos(d, x, 320 + DOT_Y - DOT_H);
        lv_obj_set_style_radius(d, DOT_H / 2, 0);
        lv_obj_set_style_border_width(d, 0, 0);
        lv_obj_set_style_pad_all(d, 0, 0);
        lv_obj_set_style_bg_color(d,
            i == active_index
                ? lv_color_hex(0xFFFFFF)   // active = white pill
                : lv_color_hex(0x444466),  // inactive = dim dot
            0);
        lv_obj_clear_flag(d, LV_OBJ_FLAG_SCROLLABLE);
        dots[i] = d;
        x += w + DOT_GAP;
    }
}