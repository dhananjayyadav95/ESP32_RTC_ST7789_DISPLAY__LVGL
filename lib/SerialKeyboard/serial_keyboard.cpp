#include "serial_keyboard.h"
#include <lv_conf.h> // or your specific LVGL driver

// Constructor
SerialKeyboard::SerialKeyboard(uint16_t w, uint16_t h, uint32_t buf)
    : width(w), height(h), buf_size(buf), esc_state(0) 
{
    buffer = new uint8_t[buf_size];
}

// Tick callback
uint32_t SerialKeyboard::lv_tick() {
    return millis();
}

// Initialize LVGL and create UI
void SerialKeyboard::begin() {
    Serial.begin(115200);

    lv_init();
    lv_tick_set_cb(lv_tick);
    lv_tft_espi_create(width, height, buffer, buf_size);

    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    // Create textarea
    textarea = lv_textarea_create(lv_screen_active());
    lv_obj_set_size(textarea, width - 20, 200);
    lv_obj_align(textarea, LV_ALIGN_CENTER, 0, 0);
    lv_textarea_set_placeholder_text(textarea, "Type from laptop...");
    lv_textarea_set_one_line(textarea, false);  // multiline
    lv_textarea_set_cursor_click_pos(textarea, true);
    lv_obj_set_style_anim_time(textarea, 500, LV_PART_CURSOR);

    // Input group
    group = lv_group_create();
    lv_group_add_obj(group, textarea);
    lv_group_focus_obj(textarea);

    // Title label
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Serial Keyboard");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);
}

// Call this inside loop()
void SerialKeyboard::update() {
    handle_serial();
    lv_timer_handler();
    delay(5);
}

// Internal serial handler
void SerialKeyboard::handle_serial() {
    while (Serial.available()) {
        char c = Serial.read();

        if (esc_state == 0) {
            if (c == 27) {        // ESC
                esc_state = 1;
            }
            else if (c == 8 || c == 127) {
                lv_textarea_delete_char(textarea);
            }
            else if (c == '\n' || c == '\r') {
                lv_textarea_add_char(textarea, '\n');
            }
            else {
                lv_textarea_add_char(textarea, c);
            }
        }
        else if (esc_state == 1) {
            if (c == '[') esc_state = 2;
            else esc_state = 0;
        }
        else if (esc_state == 2) {
            uint32_t pos = lv_textarea_get_cursor_pos(textarea);
            switch (c) {
                case 'A': lv_textarea_set_cursor_pos(textarea, pos - 1); break; // Up
                case 'B': lv_textarea_set_cursor_pos(textarea, pos + 1); break; // Down
                case 'C': lv_textarea_set_cursor_pos(textarea, pos + 1); break; // Right
                case 'D': if (pos > 0) lv_textarea_set_cursor_pos(textarea, pos - 1); break; // Left
            }
            esc_state = 0;
        }
    }
}