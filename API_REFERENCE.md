# API Reference — EPDDisplay Library

This document provides detailed documentation for every public method, type, and constant in the `EPDDisplay` class.

For a higher-level overview and installation instructions, see [README.md](README.md).

---

## Table of Contents

1. [Class Overview](#class-overview)
2. [Coordinate System](#coordinate-system)
3. [Color Model](#color-model)
4. [Font System](#font-system)
5. [Enumerations](#enumerations)
6. [Structs and Types](#structs-and-types)
7. [Constructor & Destructor](#constructor--destructor)
8. [Initialization & Hardware Control](#initialization--hardware-control)
9. [Basic Drawing](#basic-drawing)
10. [Shape Drawing](#shape-drawing)
11. [Complex Shapes](#complex-shapes)
12. [Text Rendering](#text-rendering)
13. [Clock Widgets](#clock-widgets)
14. [Performance Notes](#performance-notes)

---

## Class Overview

```cpp
class EPDDisplay { ... };  // defined in src/EPDDisplay.h
```

`EPDDisplay` is the single class that manages the Waveshare 7.5" B HD e-Paper display. It encapsulates:
- Hardware communication (bit-banged SPI)
- Two in-memory framebuffers (black plane + red plane)
- All drawing primitives from pixels to complex clock widgets
- Power management (sleep / wakeup)

**Thread Safety:** Not thread-safe. Do not call methods from multiple FreeRTOS tasks simultaneously.

**Lifecycle:**
```
EPDDisplay display(pins...)
    └─ initialize()          ← allocates ~116 KB framebuffers, sends init sequence
         ├─ fillScreen() / drawXxx() / ...   ← modifies in-memory framebuffers only
         ├─ display()        ← pushes both framebuffers to physical display (~15–20 s)
         ├─ sleep()          ← controller enters deep sleep
         ├─ wakeUp()         ← reset + re-initialize hardware (buffers preserved)
         └─ ~EPDDisplay()    ← frees framebuffer heap
```

---

## Coordinate System

- **Origin (0, 0):** Top-left corner of the display
- **X axis:** Increases to the right (0 … 879)
- **Y axis:** Increases downward (0 … 527)
- **Resolution:** 880 × 528 pixels

```
(0,0) ──────────────────── (879,0)
  │                              │
  │         880 × 528            │
  │                              │
(0,527) ─────────────── (879,527)
```

All coordinate parameters (`x`, `y`, `Xstart`, `Ystart`, etc.) use this convention. Coordinates that exceed the display boundaries are silently clipped.

**Effect of rotation on coordinates:**

| Rotation    | Logical width | Logical height |
|-------------|---------------|----------------|
| ROTATE_0    | 880           | 528            |
| ROTATE_90   | 528           | 880            |
| ROTATE_180  | 880           | 528            |
| ROTATE_270  | 528           | 880            |

> **Known issue:** `setRotation()` and `setMirror()` contain a variable shadowing bug and currently have no effect. Rotation and mirroring transforms are coded in `drawPixel()` but the setter methods do not update the member variables.

---

## Color Model

The display has three physical ink states: **White**, **Black**, and **Red**. The library uses two parallel 1-bit framebuffers:

| Logical color  | blackBuffer bit | redBuffer bit |
|----------------|-----------------|---------------|
| `WHITE`        | 1               | 1             |
| `BLACK`        | 0               | 1             |
| `RED`          | 1               | 0             |

`NULL_COLOR` (value `0`) is a special sentinel — drawing with `NULL_COLOR` as the **background** color skips background pixel writes, achieving transparent text rendering over existing content.

**Pixel priority:** Red ink dominates black ink on this hardware. If both the black channel and red channel activate the same pixel, the display shows red.

**Buffer encoding sent to display:**
- Black channel (command `0x24`): bit=1 → white, bit=0 → black
- Red channel (command `0x26`): the buffer is bitwise-inverted before sending; bit=0 in redBuffer → red pixel shown

---

## Font System

Five fixed-size bitmap fonts are provided as static members:

| Font       | Glyph width | Glyph height | Bytes/char |
|------------|-------------|--------------|------------|
| `Font8`    | 5 px        | 8 px         | 8          |
| `Font12`   | 7 px        | 12 px        | 12         |
| `Font16`   | 11 px       | 16 px        | 32         |
| `Font20`   | 14 px       | 20 px        | 40         |
| `Font24`   | 17 px       | 24 px        | 72         |

**Bitmap encoding:**
- Each glyph is stored as `height` rows, each row padded to a whole number of bytes (MSB first)
- Bit = 1 → foreground (active) pixel; Bit = 0 → background pixel
- Glyph data is stored in ASCII order starting at space (0x20) through tilde (0x7E) — 95 characters

**Extended characters:**
- An additional 45 Unicode codepoints are supported via a separate lookup table in `font_ext.cpp`
- Supported codepoints: Latin accented letters (À–ÿ subset), plus `¡`, `¿`, `°`, `±`, `€`
- Binary search over `fontExtCodepoints[]` provides O(log 45) lookup per character
- Unsupported codepoints render as the `?` character

**Custom fonts:**
To add new characters, edit `EXT_CHARS` in `tools/generate_font_bitmaps.py` and regenerate `font_ext.cpp`. To add an entirely new font size, create a new `sFONT` struct with corresponding bitmap table.

---

## Enumerations

### `COLOR`
```cpp
typedef enum {
    NULL_COLOR = 0,  // Transparent background — skips background pixel writes
    WHITE      = 1,  // White ink
    BLACK      = 2,  // Black ink
    RED        = 3,  // Red ink
} COLOR;
```

### `ROTATE`
```cpp
typedef enum {
    ROTATE_0   = 1,  // Default orientation (landscape, USB connector varies by board)
    ROTATE_90  = 2,  // 90° clockwise
    ROTATE_180 = 3,  // 180° (upside-down landscape)
    ROTATE_270 = 4,  // 270° clockwise (90° counter-clockwise)
} ROTATE;
```
> Note: Applied in `drawPixel()` but `setRotation()` is currently non-functional due to a bug.

### `MIRROR_IMAGE`
```cpp
typedef enum {
    MIRROR_NONE       = 0x00,  // No mirroring
    MIRROR_HORIZONTAL = 0x01,  // Flip left ↔ right
    MIRROR_VERTICAL   = 0x02,  // Flip top ↔ bottom
    MIRROR_ORIGIN     = 0x03,  // Both horizontal and vertical (180° point reflection)
} MIRROR_IMAGE;
```

### `LINE_STYLE`
```cpp
typedef enum {
    LINE_SOLID  = 0,  // Continuous solid line
    LINE_DOTTED = 1,  // Dotted line (dot length scales with line_width)
    // LINE_DASHED = 2  // Reserved, not yet implemented
} LINE_STYLE;
```

### `DRAW_FILL`
```cpp
typedef enum {
    DRAW_EMPTY = 0,  // Outline only
    DRAW_FULL  = 1,  // Filled solid shape
} DRAW_FILL;
```

---

## Structs and Types

### `sFONT`
```cpp
typedef struct _tFont {
    const uint8_t *table;  // Pointer to glyph bitmap data in flash
    uint16_t width;        // Glyph width in pixels
    uint16_t height;       // Glyph height in pixels
} sFONT;
```
Pass a pointer to one of the static font instances:
```cpp
&EPDDisplay::Font8
&EPDDisplay::Font12
&EPDDisplay::Font16
&EPDDisplay::Font20
&EPDDisplay::Font24
```

---

## Constructor & Destructor

### `EPDDisplay()`

```cpp
EPDDisplay(int busy_pin, int rst_pin, int dc_pin, int cs_pin, int clk_pin, int din_pin);
```

Stores pin numbers. Does **not** configure GPIO or communicate with the display — call `initialize()` separately.

**Parameters:**
- `busy_pin` — GPIO connected to the display BUSY signal (input, active LOW)
- `rst_pin` — GPIO connected to the display RST (reset) signal (output, active LOW)
- `dc_pin` — GPIO connected to Data/Command select (output; HIGH=data, LOW=command)
- `cs_pin` — GPIO connected to chip select (output, active LOW)
- `clk_pin` — GPIO for SPI clock (output)
- `din_pin` — GPIO for SPI data in / MOSI (output)

**Example:**
```cpp
EPDDisplay display(4, 16, 17, 5, 18, 23); // BUSY, RST, DC, CS, CLK, DIN
```

---

### `~EPDDisplay()`

```cpp
~EPDDisplay();
```

Frees the two framebuffer heap allocations. Does **not** send a sleep command — call `sleep()` before destruction if power management is important.

---

## Initialization & Hardware Control

### `initialize()`

```cpp
bool initialize();
```

**Description:**
Allocates framebuffers, configures GPIO pins, performs a hardware reset, and sends the full controller initialization sequence (SWRESET, MUX configuration, data entry mode, RAM address windows, temperature/waveform load). Safe to call multiple times — subsequent calls return `true` immediately without re-initializing.

**Returns:**
- `true` — initialization succeeded; display is ready
- `false` — heap allocation failed (insufficient free RAM)

**Notes:**
- Allocates 2 × 58,080 bytes (~113 KB) on the heap
- After success, `isInitialized` flag is set to `true`
- Resets rotation to `ROTATE_0` and mirror to `MIRROR_NONE` on each call

**Example:**
```cpp
if (!display.initialize()) {
    Serial.println("Not enough RAM for framebuffers!");
    while (true) {}
}
```

---

### `reset()`

```cpp
void reset();
```

**Description:**
Toggles the RST pin LOW for 2 ms then HIGH, then waits 200 ms for the controller to boot. Also clears the `isSleep` flag. Called internally by `initialize()` and `wakeUp()`.

**Notes:**
- Does not re-send the initialization command sequence — use `initialize()` for a full reset
- May be called directly if you suspect the controller is in an unknown state

---

### `clear()`

```cpp
void clear();
```

**Description:**
Clears both the in-memory framebuffers (fills them with white) **and** sends the clear command to the physical display. The display panel itself refreshes to all-white.

**Notes:**
- Requires the display to be initialized and not in sleep mode (checked via `checkDisplayReady()`)
- Takes approximately 1–2 seconds for the hardware clear command to complete
- Also calls `delay(200)` and waits for the BUSY signal

**See also:** `fillScreen()` — fills only the in-memory buffer without triggering a hardware refresh

---

### `display()`

```cpp
void display();
```

**Description:**
Sends both framebuffers to the display controller and triggers a full panel refresh. This is the only method that makes drawn content visible on the physical screen.

**Notes:**
- Requires the display to be initialized and not in sleep mode
- Takes **approximately 15–20 seconds** to complete (e-paper full refresh time)
- The BUSY signal is polled until the refresh completes
- Call this once after all drawing operations, not after each individual draw call

**Example:**
```cpp
display.fillScreen(EPDDisplay::WHITE);
display.drawString(10, 10, "Hello!", &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
display.display();   // Now visible on screen — takes ~15–20 s
```

---

### `sleep()`

```cpp
void sleep();
```

**Description:**
Sends the deep sleep command (0x10 with data 0x01) to the display controller, putting it in ultra-low power mode. Current draw drops to under 1 µA.

**Notes:**
- Has no effect if already in sleep mode
- Has no effect if not initialized
- Sets `isSleep = true`
- Does **not** clear the framebuffers — call `wakeUp()` + `display()` to re-show existing content

---

### `wakeUp()`

```cpp
void wakeUp();
```

**Description:**
If the display is in sleep mode, performs a hardware reset (`reset()`) to restore the controller to an operational state. The initialization sequence is **not** re-sent — the controller retains its configuration after a reset from sleep.

**Notes:**
- Safe to call even when not in sleep (does nothing in that case)
- After `wakeUp()`, you must call `display()` again to refresh the panel if content has changed
- The in-memory framebuffers are preserved across sleep/wakeup cycles

---

### `isInSleep()`

```cpp
bool isInSleep();
```

**Returns:** `true` if the display is currently in sleep mode, `false` otherwise.

---

## Basic Drawing

All drawing methods write to the in-memory framebuffers only. Call `display()` to make changes visible.

### `fillScreen()`

```cpp
void fillScreen(COLOR color);
```

**Description:**
Fills the entire framebuffer with a single color. This is a byte-fill operation (very fast, no per-pixel loop).

**Parameters:**
- `color` — `WHITE`, `BLACK`, `RED`, or `NULL_COLOR` (no-op)

**Example:**
```cpp
display.fillScreen(EPDDisplay::WHITE);  // White background
```

---

### `drawPixel()`

```cpp
void drawPixel(uint16_t x, uint16_t y, COLOR color);
```

**Description:**
Sets a single pixel at `(x, y)`. Applies rotation and mirroring transforms before writing to the buffer.

**Parameters:**
- `x` — X coordinate (0 … width–1)
- `y` — Y coordinate (0 … height–1)
- `color` — `WHITE`, `BLACK`, `RED`, or `NULL_COLOR` (no-op)

**Notes:**
- Out-of-bounds coordinates are silently ignored
- `NULL_COLOR` is a no-op — the pixel is not modified

**Buffer encoding:** `Addr = x/8 + y * widthByte` (widthByte = 110 for 880-pixel width)

---

### `drawBitmap()` (2 overloads)

```cpp
// Overload 1: default colors (black pixels on white background)
void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint8_t *bitmap);

// Overload 2: custom active/inactive colors
void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint8_t *bitmap, COLOR active_color, COLOR inactive_color);
```

**Description:**
Renders a 1-bit monochrome bitmap. Bit=1 in the bitmap data maps to `active_color`; bit=0 maps to `inactive_color`. Setting `inactive_color` to `NULL_COLOR` creates a transparent background.

**Parameters:**
- `x`, `y` — top-left corner of the bitmap on the display
- `width`, `height` — bitmap dimensions in pixels
- `bitmap` — pointer to bitmap data; rows are packed MSB-first, padded to byte boundaries
- `active_color` — color for set bits (1-bits); default: `BLACK`
- `inactive_color` — color for clear bits (0-bits); default: `WHITE`; use `NULL_COLOR` for transparency

**Bitmap format:**
```
Byte 0: [pixel 0][pixel 1][pixel 2][pixel 3][pixel 4][pixel 5][pixel 6][pixel 7]
Byte 1: [pixel 8]...
```
Each row is `ceil(width / 8)` bytes. Unused bits in the last byte of each row are ignored.

**Example:**
```cpp
// 8×8 checkerboard pattern
static const uint8_t checker[] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};
display.drawBitmap(100, 100, 8, 8, checker, EPDDisplay::RED, EPDDisplay::WHITE);
```

---

### `setRotation()`

```cpp
void setRotation(uint8_t rotate);
```

**Description:**
Intended to set the display rotation. Accepts `ROTATE_0`, `ROTATE_90`, `ROTATE_180`, or `ROTATE_270`.

> **Known bug:** This method contains a variable shadowing issue — the parameter `rotate` shadows the member variable `this->rotate`. The assignment `rotate = rotate` is a no-op. The member variable is never updated. As a workaround, construct the display with the desired content orientation in mind.

---

### `setMirror()`

```cpp
void setMirror(uint8_t mirror);
```

**Description:**
Intended to set display mirroring. Accepts `MIRROR_NONE`, `MIRROR_HORIZONTAL`, `MIRROR_VERTICAL`, or `MIRROR_ORIGIN`.

> **Known bug:** Same variable shadowing issue as `setRotation()`. Currently has no effect.

---

## Shape Drawing

### `drawLine()`

```cpp
void drawLine(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend,
              COLOR color, uint8_t line_width, LINE_STYLE line_style);
```

**Description:**
Draws a line from `(Xstart, Ystart)` to `(Xend, Yend)` using Bresenham's line algorithm. Supports variable thickness and dotted style.

**Parameters:**
- `Xstart`, `Ystart` — start point
- `Xend`, `Yend` — end point
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — stroke width in pixels (1 = single pixel; >1 uses `drawPoint()`)
- `line_style` — `LINE_SOLID` or `LINE_DOTTED`

**Notes:**
- Dotted pattern: 3×`line_width` pixels drawn, 1×`line_width` pixels skipped
- `line_width = 0` is silently ignored (no line drawn)

---

### `drawPoint()`

```cpp
void drawPoint(uint16_t Xpoint, uint16_t Ypoint, COLOR color, uint8_t point_width);
```

**Description:**
Draws a square "thick pixel" centered at `(Xpoint, Ypoint)`. The total size is `(2 * point_width - 1)` × `(2 * point_width - 1)` pixels.

**Parameters:**
- `Xpoint`, `Ypoint` — center of the point
- `color` — `WHITE`, `BLACK`, or `RED`
- `point_width` — half-size; `1` = 1×1 pixel, `2` = 3×3 pixels, `3` = 5×5 pixels, etc.

---

### `drawCircle()`

```cpp
void drawCircle(uint16_t Xcenter, uint16_t Ycenter, uint16_t radius,
                COLOR color, uint8_t line_width, DRAW_FILL draw_fill);
```

**Description:**
Draws a circle using the Bresenham midpoint circle algorithm. Both outline and filled modes are supported.

**Parameters:**
- `Xcenter`, `Ycenter` — center coordinates
- `radius` — radius in pixels
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — outline width in pixels (ignored for filled circles)
- `draw_fill` — `DRAW_EMPTY` (outline only) or `DRAW_FULL` (filled)

**Algorithm:** Bresenham midpoint — O(radius) iterations using integer arithmetic only.

---

### `drawRectangle()`

```cpp
void drawRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend,
                   COLOR color, uint8_t line_width, LINE_STYLE line_style, DRAW_FILL draw_Fill);
```

**Description:**
Draws an axis-aligned rectangle. For filled mode, draws horizontal scan lines from top to bottom. For outline mode, draws four lines using `drawLine()`.

**Parameters:**
- `Xstart`, `Ystart` — top-left corner
- `Xend`, `Yend` — bottom-right corner
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — stroke width in pixels
- `line_style` — `LINE_SOLID` or `LINE_DOTTED` (applies to outline mode only)
- `draw_Fill` — `DRAW_EMPTY` (outline) or `DRAW_FULL` (filled)

---

### `drawTriangle()`

```cpp
void drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t x3, uint16_t y3,
                  COLOR color, uint8_t line_width, DRAW_FILL draw_fill);
```

**Description:**
Draws a triangle with vertices at `(x1,y1)`, `(x2,y2)`, `(x3,y3)`. Filled mode uses a scanline algorithm with vertex sorting.

**Parameters:**
- `x1`/`y1`, `x2`/`y2`, `x3`/`y3` — three vertices (any order)
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — outline stroke width
- `draw_fill` — `DRAW_EMPTY` (outline) or `DRAW_FULL` (solid fill)

**Algorithm (fill):** Vertices are sorted by Y coordinate. Two-pass scanline: upper half (y1→y2) and lower half (y2→y3), interpolating edge intersections linearly.

---

### `drawEllipse()`

```cpp
void drawEllipse(uint16_t x_center, uint16_t y_center,
                 uint16_t radius_x, uint16_t radius_y,
                 COLOR color, uint8_t line_width, DRAW_FILL draw_fill);
```

**Description:**
Draws an axis-aligned ellipse using the midpoint ellipse algorithm. Setting both radii equal draws a perfect circle.

**Parameters:**
- `x_center`, `y_center` — center coordinates
- `radius_x` — horizontal semi-axis in pixels
- `radius_y` — vertical semi-axis in pixels
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — outline stroke width
- `draw_fill` — `DRAW_EMPTY` or `DRAW_FULL`

**Algorithm:** Two-region midpoint ellipse — handles the upper/lower halves with different decision parameters.

---

### `drawPolygon()`

```cpp
void drawPolygon(const uint16_t *points_x, const uint16_t *points_y, uint8_t num_points,
                 COLOR color, uint8_t line_width, DRAW_FILL draw_fill);
```

**Description:**
Draws an arbitrary polygon with `num_points` vertices. Outline mode connects vertices with lines; filled mode uses a scanline even-odd fill.

**Parameters:**
- `points_x` — array of X coordinates (length: `num_points`)
- `points_y` — array of Y coordinates (length: `num_points`)
- `num_points` — number of vertices (minimum 3; returns silently if fewer)
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — stroke width
- `draw_fill` — `DRAW_EMPTY` (outline) or `DRAW_FULL` (scanline fill)

**Algorithm (fill):** For each scanline Y, finds all edge intersections, sorts them, then fills pairs of intersections. Maximum 20 intersections per scanline (hardcoded limit).

**Example:**
```cpp
uint16_t xs[] = {100, 150, 200, 175, 125};
uint16_t ys[] = {50,  50,  100, 150, 150};
display.drawPolygon(xs, ys, 5, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
```

---

## Complex Shapes

### `drawRoundedRectangle()`

```cpp
void drawRoundedRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend,
                           uint16_t radius, COLOR color, uint8_t line_width,
                           LINE_STYLE line_style, DRAW_FILL draw_fill);
```

**Description:**
Draws a rectangle with quarter-circle corners. The straight sides connect the arc endpoints. The corner arc uses the Bresenham midpoint circle algorithm limited to each quadrant.

**Parameters:**
- `Xstart`, `Ystart` — top-left corner
- `Xend`, `Yend` — bottom-right corner
- `radius` — corner arc radius in pixels; should be ≤ half the shorter dimension
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — stroke width
- `line_style` — `LINE_SOLID` or `LINE_DOTTED`
- `draw_fill` — `DRAW_EMPTY` (outline) or `DRAW_FULL`

> **Note:** Filled rounded rectangles are not fully implemented in the current version — the `draw_fill` parameter is accepted but fill behavior may be incomplete.

---

### `drawStar()`

```cpp
void drawStar(uint16_t x_center, uint16_t y_center,
              uint16_t radius_outer, uint16_t radius_inner, uint8_t num_points,
              COLOR color, uint8_t line_width, DRAW_FILL draw_fill);
```

**Description:**
Draws an N-pointed star by alternating outer and inner radius points around the center. Vertices are computed using trigonometry; the first point is at 12 o'clock (–90°).

**Parameters:**
- `x_center`, `y_center` — center of the star
- `radius_outer` — radius to the star tips
- `radius_inner` — radius to the inner indentations (0 < inner < outer)
- `num_points` — number of points (3–10; clamped automatically)
- `color` — `WHITE`, `BLACK`, or `RED`
- `line_width` — stroke width (for outline mode)
- `draw_fill` — `DRAW_EMPTY` (outline) or `DRAW_FULL`

**Fill mode:** Draws lines from the center to each edge point, creating a spoke-fill effect.

**Example:**
```cpp
// Classic 5-pointed star
display.drawStar(200, 200, 80, 35, 5, EPDDisplay::RED, 1, EPDDisplay::DRAW_EMPTY);
```

---

## Text Rendering

### `drawChar()`

```cpp
void drawChar(uint16_t Xpoint, uint16_t Ypoint, const char Acsii_Char,
              sFONT *Font, COLOR color_foreground, COLOR color_background);
```

**Description:**
Renders a single ASCII character at `(Xpoint, Ypoint)`. The character bitmap is looked up by offset `(char - ' ') * bytes_per_glyph` in the font table.

**Parameters:**
- `Xpoint`, `Ypoint` — top-left corner of the character cell
- `Acsii_Char` — character to draw (space 0x20 through tilde 0x7E; others may produce garbage)
- `Font` — pointer to font (e.g., `&EPDDisplay::Font16`)
- `color_foreground` — color for set bits (text color)
- `color_background` — color for clear bits; use `NULL_COLOR` for transparent background

**Notes:**
- For extended characters, prefer `drawString()` which handles UTF-8 decoding
- `color_background = NULL_COLOR` skips all background pixels (faster for overlaid text)

---

### `drawString()`

```cpp
void drawString(uint16_t Xstart, uint16_t Ystart, const char *pString,
                sFONT *Font, COLOR color_foreground, COLOR color_background);
```

**Description:**
Renders a null-terminated UTF-8 string starting at `(Xstart, Ystart)`. Decodes 1-, 2-, and 3-byte UTF-8 sequences on the fly. Performs automatic line wrapping when the next character would exceed the display width; wraps back to the start column when the bottom is reached.

**Parameters:**
- `Xstart`, `Ystart` — top-left start position
- `pString` — null-terminated UTF-8 string
- `Font` — pointer to font
- `color_foreground` — text color
- `color_background` — background color; `NULL_COLOR` for transparency

**UTF-8 support:**
- 1-byte (0x00–0x7F): standard ASCII
- 2-byte (0xC0–0xDF + continuation): Latin-1 supplement, Latin Extended-A
- 3-byte (0xE0–0xEF + 2 continuations): Basic Multilingual Plane (limited by extended table)
- 4-byte sequences: silently skipped (emoji and supplementary planes not supported)

**Extended codepoints (45 total):**
`¡ ° ± ¿ À Á Â Ä Ç È É Ê Ë Í Î Ï Ñ Ó Ö Ù Ú Û Ü ß à á â ä ç è é ê ë í î ï ñ ó ö ù ú û ü ÿ €`

**Example:**
```cpp
display.drawString(10, 10, "Café & naïve résumé — €42", &EPDDisplay::Font16,
                   EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
```

---

### `drawNumber()`

```cpp
void drawNumber(uint16_t Xpoint, uint16_t Ypoint, int32_t Number,
                sFONT *Font, COLOR color_foreground, COLOR color_background);
```

**Description:**
Converts a 32-bit signed integer to a decimal string (with leading minus sign for negatives) and renders it using `drawString()`.

**Parameters:**
- `Xpoint`, `Ypoint` — top-left start position
- `Number` — integer to display (`int32_t` range: –2,147,483,648 to +2,147,483,647)
- `Font` — pointer to font
- `color_foreground`, `color_background` — colors

**Example:**
```cpp
display.drawNumber(10, 50, -273,  &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
display.drawNumber(10, 85, 100000, &EPDDisplay::Font20, EPDDisplay::RED,   EPDDisplay::NULL_COLOR);
```

---

### `drawFloat()`

```cpp
void drawFloat(uint16_t Xpoint, uint16_t Ypoint, float Number,
               sFONT *Font, COLOR color_foreground, COLOR color_background);
```

**Description:**
Formats a float with 2 decimal places using `sprintf("%.2f", Number)` and renders the resulting string.

**Parameters:**
- `Xpoint`, `Ypoint` — top-left start position
- `Number` — float value to display
- `Font`, `color_foreground`, `color_background` — see `drawString()`

**Notes:**
- Always displays exactly 2 decimal places
- Maximum string length: 255 characters (sufficient for any valid float)

---

### `drawTime()`

```cpp
void drawTime(uint16_t Xstart, uint16_t Ystart,
              uint8_t hour, uint8_t minute, uint8_t second,
              sFONT *Font, COLOR color_foreground, COLOR color_background);
```

**Description:**
Renders a time value in `HH:MM:SS` format using individual `drawChar()` calls. Digits and colons are positioned with fixed spacing derived from the font width.

**Parameters:**
- `Xstart`, `Ystart` — start position (left edge of the first digit)
- `hour` — 0–23 (displayed as-is, no 12-hour conversion)
- `minute` — 0–59
- `second` — 0–59
- `Font`, `color_foreground`, `color_background` — see `drawString()`

**Layout:** `HH:MM:SS` — total width ≈ 8 × `Font->width` (plus colon spacing adjustments)

**Example:**
```cpp
display.drawTime(10, 200, 14, 30, 0, &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
// Renders: "14:30:00"
```

---

## Clock Widgets

### `drawAnalogClock()`

```cpp
void drawAnalogClock(uint16_t x_center, uint16_t y_center, uint16_t radius,
                     uint8_t hour, uint8_t minute, uint8_t second,
                     COLOR color_face, COLOR color_hands, COLOR color_numbers,
                     bool show_seconds = true, bool show_numbers = true);
```

**Description:**
Draws a complete analog clock face with:
- Outer circle (dial)
- 12 hour tick marks (longer lines)
- 60 minute tick marks (small dots)
- Hour hand (50% of radius, 3px wide)
- Minute hand (70% of radius, 2px wide)
- Second hand (80% of radius, 1px wide, always drawn in RED)
- Optional hour numbers (1–12) using Font16
- Filled circle at center

**Parameters:**
- `x_center`, `y_center` — center of the clock face
- `radius` — outer radius of the clock face in pixels
- `hour` — hour value (0–23; automatically converted to 12-hour for hand position)
- `minute` — 0–59
- `second` — 0–59
- `color_face` — color for the dial circle and tick marks
- `color_hands` — color for hour and minute hands (second hand is always RED)
- `color_numbers` — color for hour numbers (1–12); use `NULL_COLOR` to hide numbers
- `show_seconds` — whether to draw the second hand (default: `true`)
- `show_numbers` — whether to draw hour numerals (default: `true`)

**Notes:**
- Hour hand position accounts for minute offset: at 1:30 the hour hand is halfway between 1 and 2
- Angle origin is at 12 o'clock (–90°); angles increase clockwise
- Minimum recommended radius: 60 px (for legible numbers)

**Example:**
```cpp
// Full clock at center of display, showing 10:10:30
display.drawAnalogClock(440, 264, 200, 10, 10, 30,
                        EPDDisplay::BLACK, EPDDisplay::BLACK, EPDDisplay::RED,
                        true, true);

// Minimal clock — no second hand, no numbers
display.drawAnalogClock(100, 100, 80, 3, 45, 0,
                        EPDDisplay::BLACK, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR,
                        false, false);
```

---

### `drawDigitalClock7Segment()`

```cpp
void drawDigitalClock7Segment(uint16_t x_start, uint16_t y_start,
                               uint16_t segment_width, uint16_t segment_height,
                               uint8_t hour, uint8_t minute, uint8_t second,
                               COLOR color_on, COLOR color_off,
                               bool show_seconds = true, bool format_24h = true);
```

**Description:**
Draws a 7-segment style digital clock. Each digit consists of 7 rectangular segments (a–g) rendered as filled rectangles. The clock renders digits for hours, minutes, and optionally seconds, separated by dot-style colons.

**Parameters:**
- `x_start`, `y_start` — top-left corner of the leftmost digit
- `segment_width` — width of each digit box in pixels (segment stroke = 4 px fixed)
- `segment_height` — height of each digit box in pixels
- `hour` — 0–23
- `minute` — 0–59
- `second` — 0–59
- `color_on` — color for active (lit) segments
- `color_off` — color for inactive (unlit) segments; use `NULL_COLOR` to hide them
- `show_seconds` — include the seconds digits (default: `true`)
- `format_24h` — `true` for 24-hour, `false` for 12-hour with AM/PM label (default: `true`)

**Segment layout (standard 7-segment):**
```
 _
|_|
|_|

Segments: a=top, b=top-right, c=bottom-right,
          d=bottom, e=bottom-left, f=top-left, g=middle
```

**Total width calculation:**
- 24h with seconds: `4 × (segment_width + 10) + 2 × 25` px approximately
- Spacing between digits: 10 px; colon width: 25 px

**Example:**
```cpp
// Large 24h clock with seconds
display.drawDigitalClock7Segment(50, 200, 40, 80, 14, 30, 0,
                                 EPDDisplay::BLACK, EPDDisplay::NULL_COLOR,
                                 true, true);

// Small 12h clock without seconds
display.drawDigitalClock7Segment(50, 400, 20, 40, 3, 45, 0,
                                 EPDDisplay::RED, EPDDisplay::NULL_COLOR,
                                 false, false);
```

---

## Performance Notes

### Operation Speed Reference

| Operation | Speed | Notes |
|-----------|-------|-------|
| `fillScreen()` | ~0 ms | Byte-fill of RAM buffers |
| `drawPixel()` | ~0 ms | Single RAM write |
| `drawLine()` | < 1 ms | Bresenham, O(max(dx,dy)) |
| `drawCircle()` | < 1 ms | Bresenham, O(radius) |
| `drawString()` | < 5 ms | Per character: O(width × height) |
| `drawAnalogClock()` | < 100 ms | Uses trig (float math) |
| `drawStar()` | < 50 ms | Uses trig for vertex computation |
| `display()` | **15–20 s** | Full e-paper panel refresh — hardware limited |
| `clear()` | 1–2 s | Hardware clear command + BUSY wait |
| `sleep()` | < 200 ms | SPI command + 100 ms delay |
| `wakeUp()` | ~200 ms | RST pulse + boot delay |

### Optimization Tips

1. **Batch all draws before `display()`**: Each `display()` call triggers a 15–20 second refresh. Avoid calling it in a tight loop.
2. **Use `fillScreen()` not `clear()`**: `fillScreen()` only updates RAM (instant). `clear()` also refreshes the physical display (slow). For updating displayed content: `fillScreen(WHITE)` → draw → `display()`.
3. **`NULL_COLOR` background**: Skips background pixel writes, useful when overlaying text on a colored background. Slightly faster than specifying an explicit background.
4. **Font choice**: Smaller fonts (Font8, Font12) render faster due to fewer pixels per glyph.
5. **Sleep between updates**: E-paper retains its image indefinitely with no power. Always call `sleep()` between display updates to minimize current draw.
