

#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

#include "page_manager.h"
#include "page_clock.h"
#include "page_calendar.h"
#include "page_stopwatch.h"

// DS1302 via "Rtc by Makuna" library
#include <ThreeWire.h>
#include <RtcDS1302.h>

// Display parameters
uint16_t width;
uint16_t height;
uint32_t buf_size;
uint8_t *buffer;

void screen_backlite(bool TrueFalse)
{
    pinMode(2, OUTPUT);
    digitalWrite(2, TrueFalse ? HIGH : LOW);
}

uint32_t lv_tick()
{
    return millis();
}

void initDisplay(int w = 240, int h = 320, int buf = 80)
{
    width = w;
    height = h;
    buf_size = buf * w;
    // buffer = (uint8_t*)malloc(buf_size);
    buffer = new uint8_t[buf_size];
    // Initialize display here if needed
    lv_init();
    lv_tick_set_cb(lv_tick);
    lv_tft_espi_create(width, height, buffer, buf_size);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
}

// ── Pin definitions ──────────────────────────────────────────────────────────
#define BTN_PREV 39   // Previous month button GPIO
#define BTN_NEXT 35   // Next month button GPIO
#define BTN_ACTION 34 // Select / OK button GPIO

#define RTC_CE 33
#define RTC_SCK 26
#define RTC_IO 25

// ── Debounce timing (ms) ────────────────────────────────────────────────────
#define DEBOUNCE_MS 200
#define RTC_POLL_MS 60000UL // re-read RTC every 60 s (catches midnight)

// ============================================================
//  RTC  (ThreeWire: IO, CLK, CE)
// ============================================================
ThreeWire myWire(RTC_IO, RTC_SCK, RTC_CE);
RtcDS1302<ThreeWire> Rtc(myWire);

// ── Display driver setup (adjust for your display) ──────────────────────────
// Example uses TFT_eSPI – swap for your own driver if needed.
#include <TFT_eSPI.h>
TFT_eSPI tft;

static lv_display_t *display;
static lv_color_t buf1[240 * 10];

static uint32_t lv_tick_cb() { return (uint32_t)millis(); }



struct Btn
{
    uint8_t pin;
    bool last;
    unsigned long lastTime;
};

static Btn btns[3] = {
    {BTN_PREV, HIGH, 0},
    {BTN_NEXT, HIGH, 0},
    {BTN_ACTION, HIGH, 0},
};

// Returns true exactly once on each press
static bool btnFired(Btn &b)
{
    bool cur = digitalRead(b.pin);
    if (cur == LOW && b.last == HIGH)
    {
        unsigned long now = millis();
        if (now - b.lastTime > DEBOUNCE_MS)
        {
            b.lastTime = now;
            b.last = cur;
            return true;
        }
    }
    b.last = cur;
    return false;
}

// ============================================================
//  RTC sync  — reads DS1302, pushes date to LVGL calendar
// ============================================================

// ── RTC helpers ──────────────────────────────────────────────
static void setRtcTime()
{
    // Compile-time sync — uncomment call in setup() once, then comment back
    RtcDateTime t(__DATE__, __TIME__);
    Rtc.SetDateTime(RtcDateTime(t.Epoch32Time() + 10)); // +10s for upload delay
    Serial.println("[RTC] Time SET from compile time.");
}

static void syncRtc()
{
    if (!Rtc.IsDateTimeValid())
    {
        Serial.println("[RTC] Invalid — uncomment setRtcTime() in setup()");
        return;
    }
    RtcDateTime t = Rtc.GetDateTime();
    Serial.printf("[RTC] %04u-%02u-%02u  %02u:%02u:%02u\n",
                  t.Year(), t.Month(), t.Day(),
                  t.Hour(), t.Minute(), t.Second());
    page_calendar_set_today(t.Year(), t.Month(), t.Day());
}

// ── Setup ───────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Smart Clock ===");

    // Button pins – INPUT_PULLUP so pressing pulls to GND (active LOW)
    // NOTE: GPIO 34 & 35 on ESP32 are input-only (no internal pull-up).
    //       Add a 10kΩ external pull-up resistor to 3.3V on those pins.
    pinMode(BTN_PREV, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT);   // External pull-up required
    pinMode(BTN_ACTION, INPUT); // External pull-up required

    // ── DS1302 ──────────────────────────────────────────────
    Rtc.Begin();

    Rtc.SetIsWriteProtected(false);
    if (!Rtc.GetIsRunning())
        Rtc.SetIsRunning(true);

    // ▼ Uncomment ONCE to set time, then comment out again:
    // setRtcTime();

    // ── Display ──────────────────────────────────────────────

    initDisplay(240, 320, 50);
    screen_backlite(true); // Turn on backlight

    // Pre-load the real date into screens.c globals BEFORE building the UI
    // so the very first frame already shows the correct month highlighted.
    if (Rtc.IsDateTimeValid())
    {
        RtcDateTime now = Rtc.GetDateTime();
        g_today_year = now.Year();
        g_today_month = now.Month();
        g_today_day = now.Day();
        g_cal_year = now.Year();
        g_cal_month = now.Month();
    }

    // Build the calendar UI
    // ui_init(); // calls create_screens() internally

    // Build all pages
    page_manager_init();
    syncRtc();

    Serial.println("[APP] Ready.");

    // syncRtcToCalendar();
}

// ── Loop ────────────────────────────────────────────────────────────────────

static unsigned long lastRtcPoll = 0;
static unsigned long lastClockUpd = 0;

void loop()
{
    lv_timer_handler(); // Let LVGL do its work

    // Update time label every second
    unsigned long now = millis();

    // Clock label — update every second
    if (now - lastClockUpd >= 1000)
    {
        lastClockUpd = now;
        if (Rtc.IsDateTimeValid())
        {
            RtcDateTime t = Rtc.GetDateTime();
            page_clock_update(t.Hour(), t.Minute(), t.Second(),
                              t.Day(), t.Month(), t.Year(), t.DayOfWeek());
        }
    }

    // Stopwatch — update every loop tick
    page_stopwatch_update();

    // RTC → Calendar sync every 60s (catches midnight date change)
    if (now - lastRtcPoll >= 60000UL)
    {
        lastRtcPoll = now;
        syncRtc();
    }

    // ── Buttons ──────────────────────────────────────────────
    if (btnFired(btns[0]))
        page_handle_prev_btn(); // PREV
    if (btnFired(btns[1]))
        page_next(); // NEXT — always page cycle
    if (btnFired(btns[2]))
        page_handle_action_btn(); // ACTION
}
