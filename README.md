# Waveshare 7.5" B HD e-Paper Library for ESP32

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-blue)](https://platformio.org/)
[![Arduino IDE](https://img.shields.io/badge/Arduino%20IDE-compatible-green)](https://www.arduino.cc/)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal)](https://www.arduino.cc/)

A comprehensive C++ library for the **Waveshare 7.5 inch B HD** e-Paper display (880×528 pixels, Black/White/Red), designed for ESP32 microcontrollers with the Arduino framework.

---

## Features

- **Full 880×528 resolution** — every pixel individually addressable
- **Three-color support** — Black, White, and Red on a single screen
- **Dual framebuffer rendering** — draw to RAM, flush once; no partial refresh tearing
- **Rich text rendering** — 5 built-in bitmap font sizes (8, 12, 16, 20, 24 pt)
- **UTF-8 extended characters** — 45 Latin accented glyphs plus °, ±, ¡, ¿, €
- **Complete shape library** — lines, circles, rectangles, triangles, ellipses, polygons, stars, rounded rectangles
- **Clock widgets** — analog clock face and 7-segment digital clock, both fully configurable
- **Bitmap display** — render 1-bit bitmaps with custom active/inactive colors
- **Rotation & mirroring** — 0/90/180/270° rotation and horizontal/vertical/origin mirror
- **Power management** — `sleep()` and `wakeUp()` for ultra-low standby consumption
- **Software SPI** — bit-banged SPI works with any GPIO pins, no hardware SPI conflicts
- **Debug mode** — optional serial output controlled by a build flag

---

## Hardware Requirements

### Display
- **Waveshare 7.5 inch e-Paper B HD** (SKU: xxxxxxx)
- Resolution: 880×528 pixels
- Colors: Black, White, Red (tricolor e-ink)
- Interface: SPI (4-wire + BUSY + RST)
- Operating voltage: 3.3 V

> **Important:** This library targets the **HD** variant (880×528). It is **not** compatible with the standard 7.5" B (640×384) model.

### Microcontroller
- **ESP32** (tested on NodeMCU-32S / ESP32-DevKitC)
- Minimum free heap after framebuffer allocation: ~116 KB reserved for two buffers
- Any ESP32 variant with sufficient RAM (520 KB SRAM on standard ESP32) works

### Required Connections
Six GPIO pins are needed. The default mapping (configured via `platformio.ini` build flags):

| ESP32 GPIO | EPD Pin | Signal      | Description                          |
|------------|---------|-------------|--------------------------------------|
| 4          | BUSY    | Input       | Display busy indicator (active LOW)  |
| 16         | RST     | Output      | Hardware reset (active LOW)          |
| 17         | DC      | Output      | Data / Command select                |
| 5          | CS      | Output      | Chip select (active LOW)             |
| 18         | CLK     | Output      | SPI clock (VSPI SCK)                 |
| 23         | DIN     | Output      | SPI data in / MOSI                   |
| 3.3V       | VCC     | Power       | 3.3 V supply                         |
| GND        | GND     | Ground      | Common ground                        |

> The CLK and DIN pins match ESP32's VSPI bus, but the library uses **bit-banged SPI** — you can reassign any pin freely.

### Wiring ASCII Diagram

```
ESP32                    Waveshare 7.5" B HD HAT
┌──────────┐             ┌──────────────────────┐
│    3.3V  ├─────────────┤ VCC                  │
│     GND  ├─────────────┤ GND                  │
│  GPIO 18 ├─────────────┤ CLK                  │
│  GPIO 23 ├─────────────┤ DIN (MOSI)           │
│   GPIO 5 ├─────────────┤ CS                   │
│  GPIO 17 ├─────────────┤ DC                   │
│  GPIO 16 ├─────────────┤ RST                  │
│   GPIO 4 ├─────────────┤ BUSY                 │
└──────────┘             └──────────────────────┘
```

---

## Quick Start

The following minimal example initializes the display and draws "Hello, World!" in three colors:

```cpp
#include <Arduino.h>
#include "EPDDisplay.h"

// Pin assignments — adjust to match your wiring
#define BUSY_PIN 4
#define RST_PIN  16
#define DC_PIN   17
#define CS_PIN   5
#define CLK_PIN  18
#define DIN_PIN  23

EPDDisplay display(BUSY_PIN, RST_PIN, DC_PIN, CS_PIN, CLK_PIN, DIN_PIN);

void setup() {
    Serial.begin(115200);

    // Initialize — allocates ~116 KB of framebuffers
    if (!display.initialize()) {
        Serial.println("Display init failed!");
        return;
    }

    // Fill background with white
    display.fillScreen(EPDDisplay::WHITE);

    // Draw text in different colors
    display.drawString(10, 50, "Hello, World!", &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(10, 90, "Red text here", &EPDDisplay::Font20, EPDDisplay::RED,   EPDDisplay::NULL_COLOR);
    display.drawString(10, 130, "White on black", &EPDDisplay::Font16, EPDDisplay::WHITE, EPDDisplay::BLACK);

    // Push framebuffers to the physical display (~15–20 seconds)
    display.display();

    // Enter low-power sleep mode after refresh
    display.sleep();
}

void loop() {
    // Nothing to do after the initial display
}
```

> **Note:** `display()` triggers a full panel refresh which takes approximately 15–20 seconds for e-paper technology. This is normal behavior.

---

## Basic Usage Examples

### Example 1: Drawing Shapes

```cpp
#include "EPDDisplay.h"

EPDDisplay display(4, 16, 17, 5, 18, 23); // BUSY, RST, DC, CS, CLK, DIN

void setup() {
    display.initialize();
    display.fillScreen(EPDDisplay::WHITE);

    // Outline circle in black
    display.drawCircle(100, 100, 50, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);

    // Filled circle in red
    display.drawCircle(250, 100, 40, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);

    // Filled rectangle with thick border
    display.drawRectangle(50, 200, 200, 280, EPDDisplay::BLACK, 3,
                          EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);

    // Dotted red line
    display.drawLine(300, 200, 600, 300, EPDDisplay::RED, 2, EPDDisplay::LINE_DOTTED);

    // Triangle outline
    display.drawTriangle(400, 50, 500, 180, 300, 180,
                         EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);

    display.display();
    display.sleep();
}

void loop() {}
```

### Example 2: Displaying Text and Numbers

```cpp
#include "EPDDisplay.h"

EPDDisplay display(4, 16, 17, 5, 18, 23);

void setup() {
    display.initialize();
    display.fillScreen(EPDDisplay::WHITE);

    // Font sizes: Font8, Font12, Font16, Font20, Font24
    display.drawString(10, 10,  "Font 8",  &EPDDisplay::Font8,  EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(10, 30,  "Font 12", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(10, 55,  "Font 16", &EPDDisplay::Font16, EPDDisplay::RED,   EPDDisplay::NULL_COLOR);
    display.drawString(10, 85,  "Font 20", &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(10, 120, "Font 24", &EPDDisplay::Font24, EPDDisplay::RED,   EPDDisplay::NULL_COLOR);

    // Numbers (integer and float)
    display.drawNumber(10, 180, 42,      &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawNumber(10, 215, -1337,   &EPDDisplay::Font20, EPDDisplay::RED,   EPDDisplay::NULL_COLOR);
    display.drawFloat( 10, 250, 3.14159, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // Time (HH:MM:SS)
    display.drawTime(10, 300, 14, 30, 0, &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

    // UTF-8 accented characters
    display.drawString(10, 360, "Café, naïve, résumé", &EPDDisplay::Font16,
                       EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    display.drawString(10, 390, "Symbols: °C ±5 ¿Por qué? €", &EPDDisplay::Font12,
                       EPDDisplay::RED, EPDDisplay::NULL_COLOR);

    display.display();
    display.sleep();
}

void loop() {}
```

### Example 3: Clock Widgets

```cpp
#include "EPDDisplay.h"

EPDDisplay display(4, 16, 17, 5, 18, 23);

void setup() {
    display.initialize();
    display.fillScreen(EPDDisplay::WHITE);

    // Analog clock — center (440, 264), radius 200
    display.drawAnalogClock(
        440, 264,   // Center coordinates
        200,        // Radius
        10, 30, 45, // Hour, minute, second
        EPDDisplay::BLACK,     // Dial/marks color
        EPDDisplay::BLACK,     // Hands color
        EPDDisplay::RED,       // Numbers color (NULL_COLOR to hide)
        true,       // Show second hand
        true        // Show hour numbers
    );

    // 7-segment digital clock — top-left corner, large segments
    display.drawDigitalClock7Segment(
        10, 400,    // Start position
        30, 60,     // Segment width, height
        10, 30, 45, // Hour, minute, second
        EPDDisplay::BLACK,      // Active segment color
        EPDDisplay::NULL_COLOR, // Inactive segment color (invisible)
        true,       // Show seconds
        true        // 24-hour format
    );

    display.display();
    display.sleep();
}

void loop() {}
```

### Example 4: Bitmap Display

```cpp
#include "EPDDisplay.h"

// 1-bit bitmap: 16×16 smiley face (MSB first, 1=active color)
static const uint8_t smiley[] = {
    0x07, 0xE0,  // 0000 0111 1110 0000
    0x1F, 0xF8,  // 0001 1111 1111 1000
    0x3F, 0xFC,  // 0011 1111 1111 1100
    0x7E, 0x7E,  // 0111 1110 0111 1110
    0xFF, 0xFF,  // 1111 1111 1111 1111
    0xFF, 0xFF,  // 1111 1111 1111 1111
    0xE7, 0xE7,  // 1110 0111 1110 0111  <- eyes
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xE0, 0x07,  // 1110 0000 0000 0111  <- mouth
    0xF0, 0x0F,
    0x7E, 0x7E,
    0x3F, 0xFC,
    0x1F, 0xF8,
    0x07, 0xE0,
};

EPDDisplay display(4, 16, 17, 5, 18, 23);

void setup() {
    display.initialize();
    display.fillScreen(EPDDisplay::WHITE);

    // Draw bitmap: 1-bits → RED, 0-bits → WHITE (background skipped if NULL_COLOR)
    display.drawBitmap(100, 100, 16, 16, smiley, EPDDisplay::RED, EPDDisplay::WHITE);

    // Scale effect by tiling
    for (int i = 0; i < 5; i++) {
        display.drawBitmap(200 + i * 20, 100, 16, 16, smiley,
                           EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
    }

    display.display();
    display.sleep();
}

void loop() {}
```

---

## Configuration

### Pin Assignment

Pins are configured at compile time. In PlatformIO (`platformio.ini`):

```ini
build_flags =
    -D BUSY_pin=4
    -D RST_pin=16
    -D DC_pin=17
    -D CS_pin=5
    -D CLK_pin=18
    -D DIN_pin=23
```

In the Arduino IDE demo sketch, they are hardcoded `#define` values at the top of the `.ino` file.

At runtime, pass pins to the constructor:
```cpp
EPDDisplay display(BUSY_pin, RST_pin, DC_pin, CS_pin, CLK_pin, DIN_pin);
```

### Debug Mode

Enable verbose serial output by adding `-D DEBUG` to your build flags:

```ini
build_flags =
    -D DEBUG
```

When enabled, the `Debug(x)` macro expands to `Serial.print(x)`, printing messages such as:
- `"e-Paper busy"` / `"e-Paper busy release"` during hardware waits
- `"clear EPD"` and `"display"` on each hardware operation
- Error messages for out-of-bounds coordinates

### Extended Font Generation

The library ships with 45 pre-generated extended characters. To add more:

1. Edit `EXT_CHARS` in `tools/generate_font_bitmaps.py`
2. Run the generator:
   ```bash
   python3 tools/generate_font_bitmaps.py
   # or with higher quality TTF rendering:
   pip install Pillow
   python3 tools/generate_font_bitmaps.py --font /path/to/font.ttf
   ```
3. The output `src/fonts/font_ext.cpp` is regenerated automatically.

---

## API Overview

The entire API is exposed through the single `EPDDisplay` class. See [API_REFERENCE.md](API_REFERENCE.md) for full documentation of every method.

### Initialization & Hardware Control
| Method | Description |
|--------|-------------|
| `EPDDisplay(busy, rst, dc, cs, clk, din)` | Constructor — stores pin numbers |
| `initialize()` | Allocates buffers, configures GPIO, sends init sequence |
| `reset()` | Hardware reset via RST pin |
| `clear()` | Clears both framebuffers and physical display to white |
| `display()` | Pushes both framebuffers to the physical display |
| `sleep()` | Puts the controller in deep sleep (ultra-low power) |
| `wakeUp()` | Wakes up from sleep via hardware reset |
| `isInSleep()` | Returns `true` if currently in sleep mode |

### Drawing — Basic
| Method | Description |
|--------|-------------|
| `fillScreen(color)` | Fill entire framebuffer with one color |
| `drawPixel(x, y, color)` | Set a single pixel |
| `drawBitmap(x, y, w, h, data)` | Render a 1-bit monochrome bitmap |
| `drawBitmap(x, y, w, h, data, active, inactive)` | Render bitmap with custom colors |
| `setRotation(rotate)` | Set display orientation (0/90/180/270°) |
| `setMirror(mirror)` | Set mirroring mode |

### Drawing — Shapes
| Method | Description |
|--------|-------------|
| `drawLine(x0, y0, x1, y1, color, width, style)` | Bresenham line with optional dotted style |
| `drawPoint(x, y, color, width)` | Square point / thick pixel |
| `drawCircle(cx, cy, r, color, width, fill)` | Circle (Bresenham midpoint algorithm) |
| `drawRectangle(x0, y0, x1, y1, color, width, style, fill)` | Axis-aligned rectangle |
| `drawRoundedRectangle(x0, y0, x1, y1, r, color, width, style, fill)` | Rectangle with rounded corners |
| `drawTriangle(x1, y1, x2, y2, x3, y3, color, width, fill)` | Triangle (scanline fill) |
| `drawEllipse(cx, cy, rx, ry, color, width, fill)` | Ellipse (midpoint algorithm) |
| `drawPolygon(xs, ys, n, color, width, fill)` | Arbitrary polygon (scanline fill) |
| `drawStar(cx, cy, r_out, r_in, n, color, width, fill)` | N-pointed star |

### Drawing — Text
| Method | Description |
|--------|-------------|
| `drawChar(x, y, ch, font, fg, bg)` | Single ASCII character |
| `drawString(x, y, str, font, fg, bg)` | UTF-8 string with auto line-wrap |
| `drawNumber(x, y, n, font, fg, bg)` | Integer (int32_t) |
| `drawFloat(x, y, f, font, fg, bg)` | Float with 2 decimal places |
| `drawTime(x, y, h, m, s, font, fg, bg)` | Time in HH:MM:SS format |

### Drawing — Clocks
| Method | Description |
|--------|-------------|
| `drawAnalogClock(cx, cy, r, h, m, s, ...)` | Full analog clock face with hands |
| `drawDigitalClock7Segment(x, y, sw, sh, h, m, s, ...)` | 7-segment style digital clock |

---

## Tips
- Call `sleep()` after every `display()` to reduce current draw to <1 µA
- The framebuffers persist in RAM throughout the object lifetime — no re-allocation on each draw
- `NULL_COLOR` as background skips background pixel writes, useful when drawing over existing content

---

## Known Limitations

1. **Refresh time**: Full display refresh takes approximately 15–20 seconds. This is inherent to tricolor e-paper technology and cannot be reduced.
3. **No grayscale**: Only three colors are supported — Black, White, Red.
4. **`setRotation()` / `setMirror()` bug**: These methods contain a variable shadowing bug and do **not** apply rotation or mirroring. The `drawPixel()` function *does* handle rotation/mirroring internally, but `setRotation()` and `setMirror()` fail to update the member variables. Direct assignment of `this->rotate` and `this->mirror` is needed as a workaround (see [issue tracker](https://github.com/your-username/waveshare_7in5b_HD/issues)).
5. **UTF-8 limited to 3-byte sequences**: 4-byte UTF-8 sequences (emoji, supplementary planes) are silently skipped.
6. **Extended character set**: Only 45 specific Unicode codepoints beyond ASCII are supported (Latin accented, °, ±, ¡, ¿, €). Unsupported codepoints render as `?`.
7. **Software SPI speed**: Bit-banged SPI is slower than hardware SPI. Sending the full framebuffer (116 KB) takes several seconds — this is dominated by the display's own processing time, not SPI speed.
8. **Not thread-safe**: Do not call methods from multiple FreeRTOS tasks simultaneously.
9. **`drawFloat` precision**: Always formats to exactly 2 decimal places using `sprintf("%.2f", ...)`.

---

## Contributing

Contributions are welcome!

---

## Resources

- [Waveshare 7.5" B HD Product Page](https://www.waveshare.com/7.5inch-e-paper-b.htm)
- [Waveshare Wiki — 7.5inch e-Paper B HD](https://www.waveshare.com/wiki/7.5inch_e-Paper_B_HD)
- [ESP32 Arduino Core Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [e-Paper Technology Overview (E Ink Corporation)](https://www.eink.com/technology.html)
