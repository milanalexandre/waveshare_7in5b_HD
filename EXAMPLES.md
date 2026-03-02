# Practical Examples

This file contains complete, copy-pastable examples for common use cases with the EPDDisplay library.

For API details, see [API_REFERENCE.md](API_REFERENCE.md).
For installation and setup, see [README.md](README.md).

---

## Table of Contents

1. [Hello World](#1-hello-world)
2. [Weather Station Display](#2-weather-station-display)
3. [E-Ink Name Badge](#3-e-ink-name-badge)
4. [Information Dashboard](#4-information-dashboard)
5. [Digital Clock with Deep Sleep](#5-digital-clock-with-deep-sleep)
6. [Image / Logo Display](#6-image--logo-display)
7. [Data Logger Summary Screen](#7-data-logger-summary-screen)

---

## 1. Hello World

The minimal example — initialize, draw text, refresh, sleep.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed — not enough RAM");
        return;
    }

    display.fillScreen(EPDDisplay::WHITE);

    // Black title
    display.drawString(20, 50, "Hello, World!", &EPDDisplay::Font24,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // Red subtitle
    display.drawString(20, 90, "Waveshare 7.5\" B HD", &EPDDisplay::Font16,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // Decorative line
    display.drawLine(20, 120, 860, 120, EPDDisplay::BLACK, 2, EPDDisplay::LINE_SOLID);

    // Push to screen (~15–20 seconds)
    display.display();

    // Enter low-power sleep
    display.sleep();
    Serial.println("Done.");
}

void loop() {}
```

---

## 2. Weather Station Display

Displays temperature, humidity, weather condition icon, and a timestamp. This example simulates sensor data — integrate your actual sensor library as needed.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

// Simple sun icon — 24×24 pixels (replace with your own bitmap)
static const uint8_t sunIcon[] = {
    0x00, 0x00, 0x00,
    0x00, 0x18, 0x00,
    0x00, 0x18, 0x00,
    0x20, 0x00, 0x04,
    0x10, 0x00, 0x08,
    0x08, 0x00, 0x10,
    0x07, 0xFF, 0xE0,
    0x1F, 0xFF, 0xF8,
    0x3F, 0xFF, 0xFC,
    0x7F, 0xFF, 0xFE,
    0xF8, 0x00, 0x1F,
    0xF0, 0x00, 0x0F,
    0xF0, 0x00, 0x0F,
    0xF8, 0x00, 0x1F,
    0x7F, 0xFF, 0xFE,
    0x3F, 0xFF, 0xFC,
    0x1F, 0xFF, 0xF8,
    0x07, 0xFF, 0xE0,
    0x08, 0x00, 0x10,
    0x10, 0x00, 0x08,
    0x20, 0x00, 0x04,
    0x00, 0x18, 0x00,
    0x00, 0x18, 0x00,
    0x00, 0x00, 0x00,
};

void drawWeatherScreen(float tempC, float humidity, const char *condition,
                       uint8_t hour, uint8_t minute) {
    display.fillScreen(EPDDisplay::WHITE);

    // ── Header bar ──
    display.drawRectangle(0, 0, 880, 50, EPDDisplay::BLACK, 1,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
    display.drawString(10, 12, "WEATHER STATION", &EPDDisplay::Font24,
                       EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

    // Time in top-right corner
    display.drawTime(700, 15, hour, minute, 0, &EPDDisplay::Font20,
                     EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

    // ── Temperature (large) ──
    display.drawString(30, 80, "Temperature", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(30, 110, tempC, &EPDDisplay::Font24,
                      EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(210, 115, "°C", &EPDDisplay::Font24,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // ── Humidity ──
    display.drawString(30, 180, "Humidity", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(30, 210, humidity, &EPDDisplay::Font20,
                      EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(190, 215, "%", &EPDDisplay::Font20,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // ── Condition + icon ──
    display.drawString(30, 280, "Condition", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawBitmap(30, 310, 24, 24, sunIcon,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);
    display.drawString(70, 315, condition, &EPDDisplay::Font20,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // ── Divider ──
    display.drawLine(0, 270, 880, 270, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);

    // ── Mini analog clock ──
    display.drawAnalogClock(750, 300, 120, hour, minute, 0,
                            EPDDisplay::BLACK, EPDDisplay::BLACK, EPDDisplay::RED,
                            false, true);

    // ── Footer ──
    display.drawLine(0, 490, 880, 490, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
    display.drawString(10, 498, "Updated automatically", &EPDDisplay::Font8,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed");
        return;
    }

    // Simulated sensor data — replace with real sensor reads
    float temperature = 22.5;
    float humidity    = 65.3;
    const char *condition = "Sunny";
    uint8_t hour   = 14;
    uint8_t minute = 30;

    drawWeatherScreen(temperature, humidity, condition, hour, minute);
    display.display();
    display.sleep();
}

void loop() {}
```

---

## 3. E-Ink Name Badge

A professional name badge suitable for events and conferences. Displays name, title, company, and QR code placeholder.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

void drawNameBadge(const char *firstName, const char *lastName,
                   const char *title, const char *company,
                   const char *email) {
    display.fillScreen(EPDDisplay::WHITE);

    // ── Top color bar ──
    display.drawRectangle(0, 0, 880, 80, EPDDisplay::RED, 1,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);

    // Company name in top bar
    display.drawString(20, 20, company, &EPDDisplay::Font24,
                       EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

    // ── Main content area ──
    // First name (very large)
    display.drawString(30, 110, firstName, &EPDDisplay::Font24,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // Last name (large, red)
    display.drawString(30, 155, lastName, &EPDDisplay::Font24,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // Divider
    display.drawLine(30, 205, 600, 205, EPDDisplay::BLACK, 2, EPDDisplay::LINE_SOLID);

    // Job title
    display.drawString(30, 220, title, &EPDDisplay::Font20,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // Email
    display.drawString(30, 260, email, &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // ── QR code placeholder box ──
    display.drawRoundedRectangle(660, 100, 850, 290, 10,
                                 EPDDisplay::BLACK, 2,
                                 EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
    display.drawString(690, 185, "QR CODE", &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    // In production: replace with actual QR bitmap from a QR library

    // ── Bottom border ──
    display.drawRectangle(0, 490, 880, 528, EPDDisplay::BLACK, 1,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
    display.drawString(10, 500, "www.your-company.com", &EPDDisplay::Font12,
                       EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

    // Decorative stars
    display.drawStar(830, 505, 12, 5, 5, EPDDisplay::WHITE, 1, EPDDisplay::DRAW_FULL);
}

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed");
        return;
    }

    drawNameBadge(
        "Alexandre",
        "Dupont",
        "Senior Embedded Engineer",
        "TechCorp Inc.",
        "a.dupont@techcorp.com"
    );

    display.display();
    display.sleep();
}

void loop() {}
```

---

## 4. Information Dashboard

A multi-section dashboard layout displaying system status, statistics, and a progress bar. Useful for monitoring panels, server status boards, or home automation displays.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

// Draws a labeled progress bar
void drawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     uint8_t percent, const char *label, EPDDisplay::COLOR barColor) {
    // Background (empty) bar
    display.drawRectangle(x, y, x + w, y + h, EPDDisplay::BLACK, 1,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);

    // Fill
    uint16_t fillW = (w - 2) * percent / 100;
    if (fillW > 0) {
        display.drawRectangle(x + 1, y + 1, x + 1 + fillW, y + h - 1,
                              barColor, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
    }

    // Label and percentage
    display.drawString(x, y - 16, label, &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    char pctStr[8];
    sprintf(pctStr, "%d%%", percent);
    display.drawString(x + w + 5, y + 2, pctStr, &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void drawDashboard(uint8_t hour, uint8_t minute,
                   float cpuTemp, uint8_t cpuUsage,
                   uint8_t memUsage, uint8_t diskUsage,
                   int32_t messages, bool wifiOk) {
    display.fillScreen(EPDDisplay::WHITE);

    // ── Header ──
    display.drawRectangle(0, 0, 880, 45, EPDDisplay::BLACK, 1,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
    display.drawString(10, 10, "SYSTEM DASHBOARD", &EPDDisplay::Font20,
                       EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);
    display.drawTime(700, 12, hour, minute, 0, &EPDDisplay::Font20,
                     EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

    // ── Status indicators ──
    display.drawString(20, 60, "SYSTEM STATUS", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // CPU temperature
    display.drawString(20, 90, "CPU Temp:", &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    EPDDisplay::COLOR tempColor = (cpuTemp > 70) ? EPDDisplay::RED : EPDDisplay::BLACK;
    display.drawFloat(120, 90, cpuTemp, &EPDDisplay::Font12, tempColor, EPDDisplay::NULL_COLOR);
    display.drawString(200, 90, "°C", &EPDDisplay::Font12, tempColor, EPDDisplay::NULL_COLOR);

    // Wi-Fi status
    display.drawString(20, 110, "Wi-Fi:", &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    if (wifiOk) {
        display.drawString(80, 110, "Connected", &EPDDisplay::Font12,
                           EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
        display.drawCircle(185, 116, 5, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
    } else {
        display.drawString(80, 110, "Disconnected", &EPDDisplay::Font12,
                           EPDDisplay::RED, EPDDisplay::NULL_COLOR);
    }

    // Messages pending
    display.drawString(20, 130, "Pending msgs:", &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawNumber(160, 130, messages, &EPDDisplay::Font12,
                       (messages > 0 ? EPDDisplay::RED : EPDDisplay::BLACK),
                       EPDDisplay::NULL_COLOR);

    // ── Resource usage bars ──
    display.drawLine(0, 155, 880, 155, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);
    display.drawString(20, 165, "RESOURCE USAGE", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    drawProgressBar(20, 210, 400, 20, cpuUsage,  "CPU",  EPDDisplay::BLACK);
    drawProgressBar(20, 260, 400, 20, memUsage,  "RAM",  EPDDisplay::BLACK);
    drawProgressBar(20, 310, 400, 20, diskUsage, "DISK",
                    diskUsage > 80 ? EPDDisplay::RED : EPDDisplay::BLACK);

    // ── Mini clock ──
    display.drawLine(500, 155, 500, 380, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
    display.drawAnalogClock(690, 270, 100, hour, minute, 0,
                            EPDDisplay::BLACK, EPDDisplay::BLACK, EPDDisplay::RED,
                            false, true);

    // ── Footer ──
    display.drawLine(0, 490, 880, 490, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
    display.drawString(10, 498, "ESP32 Monitor v1.0  |  Last update:", &EPDDisplay::Font8,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawTime(300, 498, hour, minute, 0, &EPDDisplay::Font8,
                     EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed");
        return;
    }

    // Simulated data — replace with real sensor / system readings
    drawDashboard(
        14, 35,     // Current time HH:MM
        62.5,       // CPU temperature °C
        45,         // CPU usage %
        72,         // RAM usage %
        23,         // Disk usage %
        3,          // Pending messages
        true        // Wi-Fi connected
    );

    display.display();
    display.sleep();
}

void loop() {}
```

---

## 5. Digital Clock with Deep Sleep

Updates a 7-segment clock display every minute using the ESP32 deep sleep feature. The display retains its image during sleep, consuming virtually no power.

```cpp
#include <Arduino.h>
#include <time.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

// NTP time sync requires Wi-Fi — simplified here with a manual time
// In production, use configTime() with your Wi-Fi credentials

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

// RTC memory persists across deep sleep cycles
RTC_DATA_ATTR uint8_t storedHour   = 0;
RTC_DATA_ATTR uint8_t storedMinute = 0;
RTC_DATA_ATTR bool    firstBoot    = true;

void drawClockScreen(uint8_t hour, uint8_t minute) {
    display.fillScreen(EPDDisplay::WHITE);

    // Title
    display.drawString(10, 30, "Current Time", &EPDDisplay::Font20,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawLine(10, 60, 870, 60, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);

    // Large 7-segment clock (HH:MM only — no seconds to reduce refresh frequency)
    display.drawDigitalClock7Segment(
        80, 150,    // Position
        60, 120,    // Segment width, height
        hour, minute, 0,
        EPDDisplay::BLACK,      // Active segments: black
        EPDDisplay::NULL_COLOR, // Inactive: invisible
        false,      // No seconds display
        true        // 24h format
    );

    // Small analog clock on the right
    display.drawAnalogClock(720, 300, 150,
                            hour, minute, 0,
                            EPDDisplay::BLACK, EPDDisplay::BLACK, EPDDisplay::RED,
                            false, true);

    // Footer
    display.drawLine(0, 490, 880, 490, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
    display.drawString(10, 500, "Updates every 60 seconds  |  Deep sleep between updates",
                       &EPDDisplay::Font8, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void setup() {
    Serial.begin(115200);

    if (firstBoot) {
        // First boot: set initial time (or sync from NTP / RTC module)
        storedHour   = 9;
        storedMinute = 0;
        firstBoot    = false;
    } else {
        // Increment time by one minute on each wake
        storedMinute++;
        if (storedMinute >= 60) {
            storedMinute = 0;
            storedHour = (storedHour + 1) % 24;
        }
    }

    if (!display.initialize()) {
        Serial.println("Init failed");
        esp_deep_sleep(60 * 1000000ULL); // Retry next minute
        return;
    }

    drawClockScreen(storedHour, storedMinute);
    display.display();
    display.sleep();

    Serial.printf("Displayed %02d:%02d. Sleeping for 60 seconds.\n",
                  storedHour, storedMinute);

    // Deep sleep for 60 seconds, then wake up and repeat
    esp_deep_sleep(60 * 1000000ULL);
}

void loop() {
    // Never reached — deep sleep restarts setup()
}
```

---

## 6. Image / Logo Display

Displays a custom 1-bit logo or icon bitmap. Use an image conversion tool (e.g., [image2cpp](https://javl.github.io/image2cpp/)) to convert a PNG/BMP to a C byte array.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

// Example: 32×32 pixel bitmap (replace with your actual bitmap)
// Generated from image2cpp with: "Sketch code", 1 bpp, Black on White
static const uint8_t myLogo[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF,  // row 0
    0x80, 0x00, 0x00, 0x01,  // row 1
    0x80, 0x00, 0x00, 0x01,  // row 2
    0x9F, 0xFF, 0xFF, 0xF1,  // row 3
    0x90, 0x00, 0x00, 0x91,  // ...
    0x90, 0xFF, 0xF0, 0x91,
    0x90, 0x80, 0x10, 0x91,
    0x90, 0x80, 0x10, 0x91,
    0x90, 0xFF, 0xF0, 0x91,
    0x90, 0x00, 0x00, 0x91,
    0x9F, 0xFF, 0xFF, 0xF1,
    0x80, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};
const uint16_t LOGO_W = 32;
const uint16_t LOGO_H = 32;

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed");
        return;
    }

    display.fillScreen(EPDDisplay::WHITE);

    // Center the logo on screen
    uint16_t logoX = (880 - LOGO_W) / 2;
    uint16_t logoY = (528 - LOGO_H) / 2 - 40;

    // Draw logo with black foreground, transparent background
    display.drawBitmap(logoX, logoY, LOGO_W, LOGO_H, myLogo,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // Caption below
    display.drawString(logoX - 40, logoY + LOGO_H + 20, "My Project Logo",
                       &EPDDisplay::Font20, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // Decorative border
    display.drawRoundedRectangle(logoX - 20, logoY - 20,
                                 logoX + LOGO_W + 20, logoY + LOGO_H + 20,
                                 10, EPDDisplay::RED, 2,
                                 EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);

    display.display();
    display.sleep();
}

void loop() {}
```

**Converting your image:**
1. Go to [https://javl.github.io/image2cpp/](https://javl.github.io/image2cpp/)
2. Upload your PNG/BMP (convert to 1-bit first in Photoshop/GIMP)
3. Set "Code output format" to "Arduino code"
4. Set "Draw mode" to "Horizontal - 1 bit per pixel"
5. Copy the generated array into your sketch

---

## 7. Data Logger Summary Screen

Displays the last 24 readings as a simple bar chart alongside statistics. Useful for sensor logging or IoT dashboards.

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

// Draw a simple bar chart
void drawBarChart(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  const float *values, uint8_t count,
                  float minVal, float maxVal,
                  EPDDisplay::COLOR barColor) {
    // Axes
    display.drawLine(x, y, x, y + h, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
    display.drawLine(x, y + h, x + w, y + h, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);

    float range = maxVal - minVal;
    if (range == 0) range = 1;

    uint16_t barW = (w - 5) / count;

    for (uint8_t i = 0; i < count; i++) {
        float normalized = (values[i] - minVal) / range;
        uint16_t barH = (uint16_t)(normalized * (h - 2));
        uint16_t bx = x + 3 + i * barW;
        uint16_t by = y + h - barH;

        if (barH > 0) {
            display.drawRectangle(bx, by, bx + barW - 2, y + h,
                                  barColor, 1, EPDDisplay::LINE_SOLID,
                                  EPDDisplay::DRAW_FULL);
        }
    }
}

void drawDataScreen(const float *hourlyTemps, uint8_t count,
                    float currentTemp, float minTemp, float maxTemp, float avgTemp,
                    uint8_t hour, uint8_t minute) {
    display.fillScreen(EPDDisplay::WHITE);

    // ── Title ──
    display.drawString(10, 10, "24h Temperature Log", &EPDDisplay::Font20,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawTime(650, 10, hour, minute, 0, &EPDDisplay::Font20,
                     EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawLine(0, 45, 880, 45, EPDDisplay::BLACK, 2, EPDDisplay::LINE_SOLID);

    // ── Current reading (large) ──
    display.drawString(20, 55, "Current:", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(170, 50, currentTemp, &EPDDisplay::Font24,
                      EPDDisplay::RED, EPDDisplay::NULL_COLOR);
    display.drawString(370, 55, "°C", &EPDDisplay::Font24,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    // ── Statistics ──
    display.drawLine(0, 110, 880, 110, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);

    display.drawString(20, 120, "Min:", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(90, 120, minTemp, &EPDDisplay::Font16,
                      EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(185, 120, "°C", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    display.drawString(250, 120, "Max:", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(320, 120, maxTemp, &EPDDisplay::Font16,
                      EPDDisplay::RED, EPDDisplay::NULL_COLOR);
    display.drawString(415, 120, "°C", &EPDDisplay::Font16,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    display.drawString(480, 120, "Avg:", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawFloat(550, 120, avgTemp, &EPDDisplay::Font16,
                      EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(645, 120, "°C", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // ── Bar chart ──
    display.drawLine(0, 155, 880, 155, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);
    display.drawString(20, 160, "24h chart:", &EPDDisplay::Font12,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    drawBarChart(20, 185, 840, 280,
                 hourlyTemps, count,
                 minTemp - 2, maxTemp + 2,
                 EPDDisplay::BLACK);

    // X-axis labels (every 4 hours)
    for (uint8_t h = 0; h < 24; h += 4) {
        char label[4];
        sprintf(label, "%02d", h);
        uint16_t lx = 20 + (h * 840 / 24);
        display.drawString(lx, 470, label, &EPDDisplay::Font8,
                           EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    }
    display.drawString(20, 483, "Hour (00h – 23h)", &EPDDisplay::Font8,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void setup() {
    Serial.begin(115200);

    if (!display.initialize()) {
        Serial.println("Init failed");
        return;
    }

    // Simulated 24h temperature data — replace with actual readings from storage
    float hourlyTemps[24] = {
        18.2, 17.9, 17.5, 17.2, 17.0, 17.3,
        18.1, 19.5, 21.2, 22.8, 23.9, 24.5,
        25.1, 25.3, 25.0, 24.7, 24.2, 23.5,
        22.8, 22.1, 21.4, 20.7, 20.1, 19.5
    };

    drawDataScreen(
        hourlyTemps, 24,
        22.8,   // current
        17.0,   // min
        25.3,   // max
        21.5,   // avg
        15, 30  // current time
    );

    display.display();
    display.sleep();
}

void loop() {}
```

---

## Tips for All Examples

### Efficient Screen Updates
```cpp
// WRONG — refreshes hardware 3 times, 15–20 s each
display.drawString(...);
display.display();
display.drawCircle(...);
display.display();
display.drawNumber(...);
display.display();

// CORRECT — one hardware refresh at the end
display.drawString(...);
display.drawCircle(...);
display.drawNumber(...);
display.display();  // Single 15–20 s refresh
```

### Clearing vs. Filling
```cpp
// clear() — slow (~2 s), also refreshes hardware display
display.clear();

// fillScreen() — instant, only updates RAM buffers
display.fillScreen(EPDDisplay::WHITE);
// Use this before drawing a new frame, then call display() once
```

### Transparent Text Overlay
```cpp
// Draw red box first
display.drawRectangle(100, 100, 300, 140, EPDDisplay::RED, 1,
                      EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);

// Overlay white text with NULL_COLOR background (no background rectangle drawn)
display.drawString(110, 110, "Label", &EPDDisplay::Font16,
                   EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);
```

### Power Management Pattern
```cpp
void refreshDisplay() {
    display.wakeUp();         // Wake from sleep (fast)
    display.fillScreen(EPDDisplay::WHITE);
    // ... draw content ...
    display.display();        // Push to screen (15–20 s)
    display.sleep();          // Re-enter sleep (<1 µA)
}
```
