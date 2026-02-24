# ESP32 RTC Calendar

### PlatformIO · LVGL v9 · DS1302 · TFT_eSPI

A real-time calendar display for ESP32 with a live clock panel on top and a full monthly calendar below. Navigate months with 3 physical buttons. Date and time are kept accurate by a DS1302 RTC module with battery backup.

---

## Screen layout

```
┌─────────────────────┐  y = 0
│      HH:MM:SS       │
│  Mon, 24 Feb 2026   │  ← Clock panel (80 px)
├─────────────────────┤  y = 80
│                     │
│     << Feb 2026 >>  │
│  Mo Tu We Th Fr Sa  │
│         1  2  3  4  │  ← Calendar (240 px)
│   5  6  7  8 …      │
│                     │
└─────────────────────┘  y = 320
```

---

## Project structure

Calendar/
├── platformio.ini          ← board, framework, libraries
├── lib/
│   ├── screens.cpp         ← LVGL screen builder (clock panel + calendar)
│   ├── ui.cpp / styles.cpp / images.cpp
│   ├── screens.h           ← calendar + clock globals & function declarations
│   ├── ui.h / actions.h / fonts.h / vars.h / styles.h / images.h / structs.h
└── src/
    ├── main.cpp            ← application: RTC, buttons, clock update loop

## Hardware wiring

### DS1302 RTC

| DS1302 pin | ESP32 GPIO |
| ---------- | ---------- |
| CE / RST   | 33         |
| CLK / SCK  | 26         |
| DAT / IO   | 25         |
| VCC        | 3.3 V      |
| GND        | GND        |

> The DS1302 keeps time from a CR2032 coin cell even when the ESP32 is unpowered.

### Buttons (active LOW — press connects pin to GND)

| Button | GPIO | Function       | Pull-up                       |
| ------ | ---- | -------------- | ----------------------------- |
| PREV   | 39   | Previous month | Internal (`INPUT_PULLUP`)   |
| NEXT   | 35   | Next month     | External 10 kΩ to 3.3 V ⚠️ |
| TODAY  | 34   | Jump to today  | External 10 kΩ to 3.3 V ⚠️ |

> ⚠️ GPIO 34 and 35 are **input-only** pins on the ESP32 — they have no internal pull-up resistor. You must add a 10 kΩ resistor between the pin and 3.3 V, otherwise the pin floats and triggers randomly.

---

## Libraries

Installed automatically by PlatformIO — no manual steps needed.

| Library             | Version | Purpose                        |
| ------------------- | ------- | ------------------------------ |
| `lvgl/lvgl`       | ^9.2.0  | UI framework & calendar widget |
| `bodmer/TFT_eSPI` | ^2.5.43 | Display driver                 |
| `makuna/RTC`      | ^2.4.2  | DS1302 date/time (ThreeWire)   |

---

## First-time setup

### Step 1 — Configure your display

Do as In platformio.ini:
build_flags =
    ; -I include ; the folder containing lvgl.h
    -I lib
    ; -I .pio/libdeps/esp32dev/lvgl/examples
    -D USER_SETUP_LOADED=1
    -D ST7789_DRIVER=1
    -D LV_USE_TFT_ESPI=1
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_MOSI=13
    -D TFT_SCLK=14
    -D TFT_CS=16
    -D TFT_DC=27
    -D TFT_RST=12
    -D LV_CONF_SKIP
    -D LV_COLOR_DEPTH=16
    -D LV_USE_LOG=1
    -D LV_FONT_MONTSERRAT_24=1
    -D LV_FONT_MONTSERRAT_16=1
    -D LV_FONT_MONTSERRAT_32=1

### Step 2 — Set the RTC time (once only)

The RTC uses your **compile time** automatically — no manual date entry needed.

In `src/main.cpp`, the function already uses:

```cpp
void setRtcTime() {
    RtcDateTime compiled(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);
}
```

**To set the clock:**

1. Uncomment the call in `setup()`:
   ```cpp
   setRtcTime();   // ← uncomment
   ```
2. Click **Upload** in PlatformIO
3. Open Serial Monitor — confirm you see:
   ```
   [RTC] Time SET to compile time: Feb 24 2026 12:30:45
   [RTC] 2026-02-24  12:30:52
   ```
4. Comment the line out again and re-upload:
   ```cpp
   // setRtcTime();   // ← comment back out
   ```

> After this the DS1302 battery keeps the time indefinitely. You only need to repeat this step if the battery dies or you replace the RTC module.

> **Tip:** If the time is slightly behind due to compile/upload delay, add a small offset:
>
> ```cpp
> RtcDateTime compiled(__DATE__, __TIME__);
> Rtc.SetDateTime(RtcDateTime(compiled.Epoch32Time() + 30)); // +30 seconds
> ```

### Step 3 — Build and upload normally

```
// setRtcTime();   ← must be commented out for normal use
```

Every subsequent upload will show the correct time from the RTC battery.

---

## How it works

### Clock panel (top 80 px)

- Updates every **1 second** directly from the DS1302
- Shows `HH:MM:SS` in large font and `Day, DD Mon YYYY` below it
- Rendered as a separate LVGL object so the calendar never overwrites it

### Calendar (y = 80, 240 px tall)

- Opens on **today's month** on boot
- **Today's date** is always highlighted
- Navigate freely with PREV / NEXT buttons
- TODAY button snaps back to the current month from anywhere

### RTC sync

- Re-reads the DS1302 every **60 seconds** in the background
- This means the date highlight updates correctly at **midnight** without a reboot

---

## Troubleshooting

| Problem                                 | Cause                           | Fix                                                                     |
| --------------------------------------- | ------------------------------- | ----------------------------------------------------------------------- |
| Time shows `00:00:00`                 | RTC not set                     | Run `setRtcTime()` once (Step 2)                                      |
| Wrong year (e.g. 2025 instead of 2026)  | Old value stored in RTC         | Re-run `setRtcTime()` with latest build                               |
| Time label flickers / disappears        | Label covered by calendar       | Label must be child of clock panel `lv_obj`, not the calendar         |
| GPIO 34/35 buttons trigger randomly     | Floating input pin              | Add 10 kΩ pull-up resistor to 3.3 V                                    |
| Display shows wrong colours             | Wrong colour order              | Toggle `TFT_RGB_ORDER` in `User_Setup.h`                            |
| Build error:`g_cal_year` not declared | Missing extern in `screens.h` | Add `extern uint16_t g_cal_year;` and `extern uint8_t g_cal_month;` |
| LVGL MEM errors                         | Heap too small                  | Increase `LV_MEM_SIZE` in `lv_conf.h`                               |

---

## Key functions (screens.h)

```cpp
// Called on boot and every 60s — reads RTC, updates today highlight
void calendar_set_today(uint16_t year, uint8_t month, uint8_t day);

void calendar_prev_month(void);   // PREV button
void calendar_next_month(void);   // NEXT button
void calendar_goto_today(void);   // TODAY button

// Global widget pointers (update labels directly from main.cpp)
extern lv_obj_t *g_calendar;
extern lv_obj_t *g_time_label;
extern lv_obj_t *g_date_label;
extern uint16_t  g_today_year;
extern uint8_t   g_today_month;
extern uint8_t   g_today_day;
```
Video demo: <video src="Media\calender working video.mp4" controls></video>