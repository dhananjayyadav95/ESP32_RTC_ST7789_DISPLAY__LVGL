

#include <Arduino.h>
#include <lvgl.h>
#include "ui.h"
#include "screens.h"

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

void initDisplay(int w = 240, int h = 320, int buf = 50)
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
#define BTN_SELECT 34 // Select / OK button GPIO

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

// ── Button state tracking ───────────────────────────────────────────────────
static unsigned long lastPressTime[3] = {0, 0, 0};

/**
 * Read a button with debounce.
 * Returns true on the falling edge (press), false otherwise.
 * index: 0 = PREV, 1 = NEXT, 2 = SELECT
 */
bool buttonPressed(uint8_t pin, uint8_t index)
{
    if (digitalRead(pin) == LOW)
    {
        unsigned long now = millis();
        if (now - lastPressTime[index] > DEBOUNCE_MS)
        {
            lastPressTime[index] = now;
            return true;
        }
    }
    return false;
}

// ============================================================
//  RTC sync  — reads DS1302, pushes date to LVGL calendar
// ============================================================
void syncRtcToCalendar()
{
    if (!Rtc.IsDateTimeValid())
    {
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
void setRtcTime()
{
    //                           YYYY   MM  DD  HH  MM  SS
    RtcDateTime compiled(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);
    Serial.println("[RTC] Time has been SET.");
}

// ── Setup ───────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);

    // Button pins – INPUT_PULLUP so pressing pulls to GND (active LOW)
    // NOTE: GPIO 34 & 35 on ESP32 are input-only (no internal pull-up).
    //       Add a 10kΩ external pull-up resistor to 3.3V on those pins.
    pinMode(BTN_PREV, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT);   // External pull-up required
    pinMode(BTN_SELECT, INPUT); // External pull-up required

    // ── DS1302 ──────────────────────────────────────────────
    Rtc.Begin();

    // ▼ Uncomment ONCE to set time, then comment out again:
    setRtcTime();

    if (!Rtc.GetIsRunning())
    {
        Serial.println("[RTC] Was stopped – starting now.");
        Rtc.SetIsRunning(true);
    }
    Rtc.SetIsWriteProtected(false);

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
    ui_init(); // calls create_screens() internally
}

static unsigned long lastRtcPoll = 0;
// ── Clock label update (every second) ───────────────────────
static unsigned long lastClockUpdate = 0;

// ── Loop ────────────────────────────────────────────────────────────────────
void loop()
{
    lv_timer_handler(); // Let LVGL do its work

    // Update time label every second
    unsigned long now = millis();

    // inside loop():
    if (now - lastClockUpdate >= 1000)
    {
        lastClockUpdate = now;
        if (Rtc.IsDateTimeValid())
        {
            RtcDateTime t = Rtc.GetDateTime();

            // Time
            if (g_time_label != NULL)
            {
                char timeBuf[12];
                snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u:%02u",
                         t.Hour(), t.Minute(), t.Second());
                lv_label_set_text(g_time_label, timeBuf);
            }

            // Date  e.g.  Mon, 24 Feb 2026
            if (g_date_label != NULL)
            {
                const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
                const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                char dateBuf[32];
                snprintf(dateBuf, sizeof(dateBuf), "%s, %02u %s %04u",
                         days[t.DayOfWeek()],
                         t.Day(),
                         months[t.Month() - 1],
                         t.Year());
                lv_label_set_text(g_date_label, dateBuf);
            }
        }
    }

    // Poll RTC every 10 seconds (10000 ms)
    if (now - lastRtcPoll >= RTC_POLL_MS)
    {
        lastRtcPoll = now;
        syncRtcToCalendar();
    }

    // ── Previous month button ──────────────────────────────────────────────
    if (buttonPressed(BTN_PREV, 0))
    {
        Serial.println("BTN_PREV pressed → previous month");
        calendar_prev_month();
    }

    // ── Next month button ──────────────────────────────────────────────────
    if (buttonPressed(BTN_NEXT, 1))
    {
        Serial.println("BTN_NEXT pressed → next month");
        calendar_next_month();
    }

    // ── Select / OK button ────────────────────────────────────────────────
    if (buttonPressed(BTN_SELECT, 2))
    {
        Serial.println("BTN_SELECT pressed → select today");
        calendar_goto_today();
        // Add your own selection logic here, e.g. read the focused date:
        // lv_calendar_date_t d;
        // if (lv_calendar_get_pressed_date(g_calendar, &d)) { ... }
    }

    delay(5); // ~200 Hz loop, plenty for button polling
}
