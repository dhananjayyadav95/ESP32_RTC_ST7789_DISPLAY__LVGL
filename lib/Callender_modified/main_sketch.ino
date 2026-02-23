/**
 * ============================================================
 *  ESP32 Real-Time Calendar  |  LVGL + DS1302 RTC
 * ============================================================
 *
 *  Hardware connections
 *  ─────────────────────────────────────────────
 *  DS1302 RTC:
 *    CE  (RST)  →  GPIO 33
 *    CLK (SCK)  →  GPIO 26
 *    DAT (IO)   →  GPIO 25
 *    VCC        →  3.3V
 *    GND        →  GND
 *
 *  Buttons (active LOW — button connects pin to GND):
 *    BTN_PREV   →  GPIO 0   (built-in pull-up OK)
 *    BTN_NEXT   →  GPIO 35  (add 10kΩ pull-up to 3.3V — input-only pin)
 *    BTN_TODAY  →  GPIO 34  (add 10kΩ pull-up to 3.3V — input-only pin)
 *
 *  Button behaviour:
 *    PREV  → go to previous month
 *    NEXT  → go to next month
 *    TODAY → jump back to today's month (today stays highlighted always)
 *
 *  Required libraries (install via Arduino Library Manager):
 *    • "Rtc by Makuna"  (search "Rtc Makuna")
 *    • LVGL             >= v9.x
 *    • TFT_eSPI         (configure your display in User_Setup.h)
 *
 *  EEZ Studio calendar UI files live in the Callender/ subfolder.
 * ============================================================
 */

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// DS1302 via "Rtc by Makuna" library
#include <ThreeWire.h>
#include <RtcDS1302.h>

// EEZ Studio generated UI
#include "Callender/ui.h"
#include "Callender/screens.h"

// ── Pin definitions ──────────────────────────────────────────────────────────
#define RTC_CE    33
#define RTC_SCK   26
#define RTC_IO    25

#define BTN_PREV   0    // Previous month  — INPUT_PULLUP (internal)
#define BTN_NEXT   35   // Next month      — needs external 10kΩ to 3.3V
#define BTN_TODAY  34   // Go to today     — needs external 10kΩ to 3.3V

// ── Display ──────────────────────────────────────────────────────────────────
#define DISP_W  240
#define DISP_H  320

// ── Timing ───────────────────────────────────────────────────────────────────
#define DEBOUNCE_MS   200
#define RTC_POLL_MS   60000UL   // re-read RTC every 60 s (catches midnight)

// ============================================================
//  RTC  (ThreeWire: IO, CLK, CE)
// ============================================================
ThreeWire myWire(RTC_IO, RTC_SCK, RTC_CE);
RtcDS1302<ThreeWire> Rtc(myWire);

// ============================================================
//  Display + LVGL
// ============================================================
TFT_eSPI tft;
static lv_color_t disp_buf[DISP_W * 10];
static lv_display_t *lvDisplay;

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_display_flush_ready(disp);
}

// ============================================================
//  Button debounce
// ============================================================
static unsigned long lastPress[3] = {0, 0, 0};

bool btnPressed(uint8_t pin, uint8_t idx) {
    if (digitalRead(pin) == LOW) {
        unsigned long now = millis();
        if (now - lastPress[idx] > DEBOUNCE_MS) {
            lastPress[idx] = now;
            return true;
        }
    }
    return false;
}

// ============================================================
//  RTC sync  — reads DS1302, pushes date to LVGL calendar
// ============================================================
void syncRtcToCalendar() {
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("[RTC] Date/time not valid! Check battery or run setRtcTime().");
        return;
    }
    RtcDateTime now = Rtc.GetDateTime();
    Serial.printf("[RTC] %04u-%02u-%02u  %02u:%02u:%02u\n",
                  now.Year(), now.Month(), now.Day(),
                  now.Hour(), now.Minute(), now.Second());

    calendar_set_today(now.Year(), now.Month(), now.Day());
}

// ============================================================
//  Call this ONCE to program the DS1302 (new module / flat battery).
//  1. Edit the date/time below.
//  2. Uncomment the call in setup().
//  3. Upload, run once, then comment it out and re-upload.
// ============================================================
void setRtcTime() {
    //                           YYYY   MM  DD  HH  MM  SS
    RtcDateTime compiled(2025,   2,  23, 12,  0,  0);
    Rtc.SetDateTime(compiled);
    Serial.println("[RTC] Time has been SET.");
}

// ============================================================
//  setup()
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 RTC Calendar ===");

    // Button pins
    pinMode(BTN_PREV,  INPUT_PULLUP); // GPIO 0  – internal pull-up
    pinMode(BTN_NEXT,  INPUT);        // GPIO 35 – external 10kΩ pull-up required
    pinMode(BTN_TODAY, INPUT);        // GPIO 34 – external 10kΩ pull-up required

    // ── DS1302 ──────────────────────────────────────────────
    Rtc.Begin();

    // ▼ Uncomment ONCE to set time, then comment out again:
    // setRtcTime();

    if (!Rtc.GetIsRunning()) {
        Serial.println("[RTC] Was stopped – starting now.");
        Rtc.SetIsRunning(true);
    }
    Rtc.SetIsWriteProtected(false);

    // ── TFT ─────────────────────────────────────────────────
    tft.begin();
    tft.setRotation(0);

    // ── LVGL ────────────────────────────────────────────────
    lv_init();
    lvDisplay = lv_display_create(DISP_W, DISP_H);
    lv_display_set_flush_cb(lvDisplay, my_disp_flush);
    lv_display_set_buffers(lvDisplay, disp_buf, NULL, sizeof(disp_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Pre-load the real date into screens.c globals BEFORE building the UI
    // so the very first frame already shows the correct month highlighted.
    if (Rtc.IsDateTimeValid()) {
        RtcDateTime now = Rtc.GetDateTime();
        g_today_year  = now.Year();
        g_today_month = now.Month();
        g_today_day   = now.Day();
        g_cal_year    = now.Year();
        g_cal_month   = now.Month();
    }

    // Build UI (create_screen_main reads g_today_* globals above)
    ui_init();

    // Final sync to make sure highlight is applied to the widget
    syncRtcToCalendar();

    Serial.println("[APP] Ready.");
}

// ============================================================
//  loop()
// ============================================================
static unsigned long lastRtcPoll = 0;

void loop() {
    lv_timer_handler();

    // Periodic RTC re-read so "today" highlight is correct at midnight
    unsigned long now = millis();
    if (now - lastRtcPoll >= RTC_POLL_MS) {
        lastRtcPoll = now;
        syncRtcToCalendar();
    }

    // Previous month
    if (btnPressed(BTN_PREV, 0)) {
        Serial.println("[BTN] < Previous");
        calendar_prev_month();
    }

    // Next month
    if (btnPressed(BTN_NEXT, 1)) {
        Serial.println("[BTN] > Next");
        calendar_next_month();
    }

    // Jump to today
    if (btnPressed(BTN_TODAY, 2)) {
        Serial.println("[BTN] ■ Today");
        calendar_goto_today();
    }

    delay(5);
}
