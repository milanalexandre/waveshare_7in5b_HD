#include "EPDDisplay.h"
#include <math.h>

// Lookup table for segments of each digit (0-9)
// Segments: a,b,c,d,e,f,g (bits 0-6)
static const uint8_t SEGMENT_PATTERNS[10] = {
    0b00111111, // 0: a,b,c,d,e,f
    0b00000110, // 1: b,c
    0b01011011, // 2: a,b,g,e,d
    0b01001111, // 3: a,b,g,c,d
    0b01100110, // 4: f,g,b,c
    0b01101101, // 5: a,f,g,c,d
    0b01111101, // 6: a,f,g,e,d,c
    0b00000111, // 7: a,b,c
    0b01111111, // 8: a,b,c,d,e,f,g
    0b01101111  // 9: a,b,c,d,f,g
};

/**
 * @brief Draw an analog clock
 */
void EPDDisplay::drawAnalogClock(uint16_t x_center, uint16_t y_center, uint16_t radius, uint8_t hour, uint8_t minute, uint8_t second, COLOR color_face, COLOR color_hands, COLOR color_numbers, bool show_seconds, bool show_numbers)
{
  // Draw the clock circle
  drawCircle(x_center, y_center, radius, color_face, 2, DRAW_EMPTY);

  // Draw hour marks
  for (int i = 0; i < 12; i++)
  {
    float angle = (i * 30.0 - 90.0) * M_PI / 180.0; // -90° to start at 12 o'clock
    uint16_t x1 = x_center + (radius - 10) * cos(angle);
    uint16_t y1 = y_center + (radius - 10) * sin(angle);
    uint16_t x2 = x_center + (radius - 5) * cos(angle);
    uint16_t y2 = y_center + (radius - 5) * sin(angle);

    drawLine(x1, y1, x2, y2, color_face, 2, LINE_SOLID);

    // Draw numbers if requested
    if (show_numbers)
    {
      uint16_t num_x = x_center + (radius - 25) * cos(angle) - 6;
      uint16_t num_y = y_center + (radius - 25) * sin(angle) - 8;
      int hour_num = (i == 0) ? 12 : i;
      drawNumber(num_x, num_y, hour_num, &Font16, color_numbers, NULL_COLOR);
    }
  }

  // Draw minute marks (smaller points)
  for (int i = 0; i < 60; i++)
  {
    if (i % 5 != 0) // Skip hour marks
    {
      float angle = (i * 6.0 - 90.0) * M_PI / 180.0;
      uint16_t x1 = x_center + (radius - 8) * cos(angle);
      uint16_t y1 = y_center + (radius - 8) * sin(angle);
      drawPoint(x1, y1, color_face, 1);
    }
  }

  // Convert time to 12-hour format if needed
  uint8_t hour_12 = hour % 12;

  // Calculate hand angles
  float hour_angle = ((hour_12 * 30.0) + (minute * 0.5) - 90.0) * M_PI / 180.0;
  float minute_angle = ((minute * 6.0) - 90.0) * M_PI / 180.0;
  float second_angle = ((second * 6.0) - 90.0) * M_PI / 180.0;

  // Draw hour hand (shortest and thickest)
  uint16_t hour_length = radius * 0.5;
  uint16_t hour_x = x_center + hour_length * cos(hour_angle);
  uint16_t hour_y = y_center + hour_length * sin(hour_angle);
  drawLine(x_center, y_center, hour_x, hour_y, color_hands, 3, LINE_SOLID);

  // Draw minute hand (longer and less thick)
  uint16_t minute_length = radius * 0.7;
  uint16_t minute_x = x_center + minute_length * cos(minute_angle);
  uint16_t minute_y = y_center + minute_length * sin(minute_angle);
  drawLine(x_center, y_center, minute_x, minute_y, color_hands, 2, LINE_SOLID);

  // Draw second hand if requested (longest and thinnest)
  if (show_seconds)
  {
    uint16_t second_length = radius * 0.8;
    uint16_t second_x = x_center + second_length * cos(second_angle);
    uint16_t second_y = y_center + second_length * sin(second_angle);
    drawLine(x_center, y_center, second_x, second_y, RED, 1, LINE_SOLID);
  }

  // Draw clock center
  drawCircle(x_center, y_center, 5, color_hands, 1, DRAW_FULL);
}

/**
 * @brief Draw a 7-segment digital clock
 */
void EPDDisplay::drawDigitalClock7Segment(uint16_t x_start, uint16_t y_start, uint16_t segment_width, uint16_t segment_height, uint8_t hour, uint8_t minute, uint8_t second, COLOR color_on, COLOR color_off, bool show_seconds, bool format_24h)
{
  uint16_t x = x_start;
  uint16_t digit_spacing = segment_width + 10;
  uint16_t colon_spacing = 15;

  // Format time according to desired format
  uint8_t display_hour = format_24h ? hour : ((hour == 0) ? 12 : ((hour > 12) ? hour - 12 : hour));

  // Draw hours
  if (format_24h || display_hour >= 10)
  {
    draw7SegmentDigit(x, y_start, display_hour / 10, segment_width, segment_height, color_on, color_off);
    x += digit_spacing;
  }
  draw7SegmentDigit(x, y_start, display_hour % 10, segment_width, segment_height, color_on, color_off);
  x += digit_spacing;

  // Draw colon ":"
  uint16_t colon_y1 = y_start + segment_height / 3;
  uint16_t colon_y2 = y_start + (2 * segment_height) / 3;
  drawPoint(x + colon_spacing / 2, colon_y1, color_on, 3);
  drawPoint(x + colon_spacing / 2, colon_y2, color_on, 3);
  x += colon_spacing + 10;

  // Draw minutes
  draw7SegmentDigit(x, y_start, minute / 10, segment_width, segment_height, color_on, color_off);
  x += digit_spacing;
  draw7SegmentDigit(x, y_start, minute % 10, segment_width, segment_height, color_on, color_off);
  x += digit_spacing;

  // Draw seconds if requested
  if (show_seconds)
  {
    // Draw colon ":"
    drawPoint(x + colon_spacing / 2, colon_y1, color_on, 3);
    drawPoint(x + colon_spacing / 2, colon_y2, color_on, 3);
    x += colon_spacing + 10;

    draw7SegmentDigit(x, y_start, second / 10, segment_width, segment_height, color_on, color_off);
    x += digit_spacing;
    draw7SegmentDigit(x, y_start, second % 10, segment_width, segment_height, color_on, color_off);
  }

  // Display AM/PM for 12-hour format
  if (!format_24h)
  {
    x += 20;
    const char *ampm = (hour < 12) ? "AM" : "PM";
    drawString(x, y_start + segment_height / 4, ampm, &Font16, color_on, NULL_COLOR);
  }
}

/**
 * @brief Draw a 7-segment digit
 */
void EPDDisplay::draw7SegmentDigit(uint16_t x, uint16_t y, uint8_t digit, uint16_t segment_width, uint16_t segment_height, COLOR color_on, COLOR color_off)
{
  if (digit > 9)
    return;

  uint8_t pattern = SEGMENT_PATTERNS[digit];

  // Draw each segment according to the pattern
  for (int i = 0; i < 7; i++)
  {
    COLOR seg_color = (pattern & (1 << i)) ? color_on : color_off;
    if (seg_color != NULL_COLOR)
    {
      draw7Segment(x, y, i, segment_width, segment_height, seg_color);
    }
  }
}

/**
 * @brief Draw an individual segment of a 7-segment display
 */
void EPDDisplay::draw7Segment(uint16_t x, uint16_t y, uint8_t segment_type, uint16_t segment_width, uint16_t segment_height, COLOR color)
{
  uint16_t thickness = 4; // Segment thickness
  uint16_t half_height = segment_height / 2;

  switch (segment_type)
  {
  case 0: // Segment a (top)
    drawRectangle(x + thickness, y, x + segment_width - thickness, y + thickness, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 1: // Segment b (top right)
    drawRectangle(x + segment_width - thickness, y + thickness, x + segment_width, y + half_height, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 2: // Segment c (bottom right)
    drawRectangle(x + segment_width - thickness, y + half_height, x + segment_width, y + segment_height - thickness, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 3: // Segment d (bottom)
    drawRectangle(x + thickness, y + segment_height - thickness, x + segment_width - thickness, y + segment_height, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 4: // Segment e (bottom left)
    drawRectangle(x, y + half_height, x + thickness, y + segment_height - thickness, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 5: // Segment f (top left)
    drawRectangle(x, y + thickness, x + thickness, y + half_height, color, 1, LINE_SOLID, DRAW_FULL);
    break;

  case 6: // Segment g (middle)
    drawRectangle(x + thickness, y + half_height - thickness / 2, x + segment_width - thickness, y + half_height + thickness / 2, color, 1, LINE_SOLID, DRAW_FULL);
    break;
  }
}