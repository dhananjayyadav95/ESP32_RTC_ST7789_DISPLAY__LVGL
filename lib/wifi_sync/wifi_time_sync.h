/**
 * wifi_time_sync.h  —  NTP sync via FreeRTOS task (no watchdog reset)
 * ─────────────────────────────────────────────────────────────────────
 * Runs WiFi + NTP on Core 0 as a background task.
 * Main app (LVGL) runs normally on Core 1 — no blocking, no WDT crash.
 *
 * Usage in main.cpp setup():
 *   startNTPSync();   // non-blocking, returns immediately
 *
 * No extra libraries needed — WiFi.h and time.h are part of ESP32 core.
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>

// ── WiFi credentials ──────────────────────────────────────────
#define WIFI_SSID      "YourWiFiName"       // ← change this
#define WIFI_PASSWORD  "YourWiFiPassword"   // ← change this

// ── NTP settings ──────────────────────────────────────────────
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      19800  // IST = UTC+5:30 = 5.5×3600
                                   // UTC+0  → 0
                                   // UTC+1  → 3600
                                   // UTC-5  → -18000
#define DAYLIGHT_OFFSET_SEC 0      // 3600 if your region uses DST

// ── Timeouts ──────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS  15000     // 15 s to connect
#define NTP_TIMEOUT_MS   10000     // 10 s to get NTP response

// ── Status flags (read from main loop if needed) ──────────────
volatile bool ntpSyncDone    = false;
volatile bool ntpSyncSuccess = false;

// forward declaration — Rtc defined in main.cpp
extern RtcDS1302<ThreeWire> Rtc;

// ─────────────────────────────────────────────────────────────
//  The actual sync logic — runs inside a FreeRTOS task on Core 0
// ─────────────────────────────────────────────────────────────
static void ntpSyncTask(void *pvParameters) {
    Serial.println("[NTP] Task started on Core 0");

    // ── Connect WiFi ────────────────────────────────────────
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  // ← reduce from 20dBm to 8.5dBm

    Serial.print("[NTP] Connecting to WiFi");

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_TIMEOUT_MS) {
            Serial.println("\n[NTP] WiFi timeout — sync skipped.");
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            ntpSyncDone    = true;
            ntpSyncSuccess = false;
            vTaskDelete(NULL);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(500));  // yield to RTOS — never use delay() in tasks
        Serial.print(".");
    }
    Serial.printf("\n[NTP] WiFi connected — IP: %s\n",
                  WiFi.localIP().toString().c_str());

    // ── Fetch NTP time ───────────────────────────────────────
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    Serial.print("[NTP] Waiting for NTP");
    struct tm timeInfo;
    start = millis();
    while (!getLocalTime(&timeInfo)) {
        if (millis() - start > NTP_TIMEOUT_MS) {
            Serial.println("\n[NTP] NTP timeout — sync skipped.");
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            ntpSyncDone    = true;
            ntpSyncSuccess = false;
            vTaskDelete(NULL);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }
    Serial.println(" OK");

    Serial.printf("[NTP] Time: %04d-%02d-%02d  %02d:%02d:%02d\n",
                  timeInfo.tm_year + 1900,
                  timeInfo.tm_mon  + 1,
                  timeInfo.tm_mday,
                  timeInfo.tm_hour,
                  timeInfo.tm_min,
                  timeInfo.tm_sec);

    // ── Write to DS1302 ──────────────────────────────────────
    RtcDateTime ntpTime(
        (uint16_t)(timeInfo.tm_year + 1900),
        (uint8_t) (timeInfo.tm_mon  + 1),
        (uint8_t)  timeInfo.tm_mday,
        (uint8_t)  timeInfo.tm_hour,
        (uint8_t)  timeInfo.tm_min,
        (uint8_t)  timeInfo.tm_sec
    );

    Rtc.SetIsWriteProtected(false);
    Rtc.SetIsRunning(true);
    Rtc.SetDateTime(ntpTime);

    Serial.println("[NTP] DS1302 updated successfully!");

    // ── Disconnect WiFi to save power ────────────────────────
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[NTP] WiFi disconnected.");

    ntpSyncDone    = true;
    ntpSyncSuccess = true;

    vTaskDelete(NULL);   // task cleans itself up — never return from a task
}

// ─────────────────────────────────────────────────────────────
//  Call this in setup() — returns IMMEDIATELY, syncs in background
// ─────────────────────────────────────────────────────────────
void startNTPSync() {
    xTaskCreatePinnedToCore(
        ntpSyncTask,   // function to run
        "ntp_sync",    // task name (for debugging)
        8192,          // stack size — WiFi needs ~4-6 kB minimum
        NULL,          // parameter passed to task
        1,             // priority 1 (low) — won't starve LVGL
        NULL,          // task handle (not needed)
        0              // Core 0 — LVGL/Arduino runs on Core 1
    );
    Serial.println("[NTP] Sync task launched in background.");
}
