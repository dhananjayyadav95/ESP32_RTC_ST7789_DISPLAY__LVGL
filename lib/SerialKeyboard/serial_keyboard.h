#ifndef SERIAL_KEYBOARD_H
#define SERIAL_KEYBOARD_H

#include <Arduino.h>
#include <lvgl.h>

class SerialKeyboard {
public:
    SerialKeyboard(uint16_t width = 240, uint16_t height = 320, uint32_t buf_size = 240 * 50);

    void begin();
    void update();  // call this in loop

private:
    uint16_t width;
    uint16_t height;
    uint32_t buf_size;
    uint8_t* buffer;

    lv_obj_t *textarea;
    lv_group_t *group;

    uint8_t esc_state;

    static uint32_t lv_tick();  // LVGL tick callback

    void handle_serial();
};

#endif