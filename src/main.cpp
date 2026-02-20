#include <Arduino.h>
#include <lvgl.h>


#define BUF_SIZE 240 * 50
uint8_t lv_buffer[BUF_SIZE];

/* Tick source, tell LVGL how much time (milliseconds) has passed */
static uint32_t my_tick(void)
{
    return millis();
}

extern "C" void lv_example_calendar_1(void);
extern "C" void lv_example_keyboard_1(void);


void setup() {

    /* Initialize LVGL */
    lv_init();
    /* Set the tick callback */
    lv_tick_set_cb(my_tick);
    /* Initialize the display driver */
    // lv_lovyan_gfx_create(320, 480, lv_buffer, BUF_SIZE, true);
    lv_tft_espi_create(240, 320, lv_buffer, BUF_SIZE);
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello PlatformIO, I'm LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0 );
    lv_example_calendar_1();
    lv_obj_clean(lv_screen_active());
    lv_example_keyboard_1();

}

void loop() {
    lv_timer_handler(); // Update the UI-
    delay(5);
}