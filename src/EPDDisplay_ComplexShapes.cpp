#include "EPDDisplay.h"
#include <cmath>

void EPDDisplay::drawRoundedRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t radius, COLOR color, uint8_t line_width, LINE_STYLE line_style, DRAW_FILL draw_fill)
{
  if (Xstart > width || Ystart > height ||
      Xend > width || Yend > height)
  {
    Debug("Input exceeds the normal display range\r\n");
    return;
  }

  drawLine(Xstart + radius, Ystart, Xend - radius, Ystart, color, line_width, line_style);
  drawLine(Xstart, Ystart + radius, Xstart, Yend - radius, color, line_width, line_style);
  drawLine(Xend, Yend - radius, Xend, Ystart + radius, color, line_width, line_style);
  drawLine(Xend - radius, Yend, Xstart + radius, Yend, color, line_width, line_style);

  // Draw a circle from(0, R) as a starting point
  int16_t XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = radius;

  int16_t Esp = 3 - (radius << 1);

  while (XCurrent <= YCurrent)
  {
    if ((line_style == EPDDisplay::LINE_SOLID) || (XCurrent % 2 == 0))
    {
      drawPoint(Xend - radius + XCurrent, Yend - radius + YCurrent, color, line_width);     // 1
      drawPoint(Xstart + radius - XCurrent, Yend - radius + YCurrent, color, line_width);   // 2
      drawPoint(Xstart + radius - YCurrent, Yend - radius + XCurrent, color, line_width);   // 3
      drawPoint(Xstart + radius - YCurrent, Ystart + radius - XCurrent, color, line_width); // 4
      drawPoint(Xstart + radius - XCurrent, Ystart + radius - YCurrent, color, line_width); // 5
      drawPoint(Xend - radius + XCurrent, Ystart + radius - YCurrent, color, line_width);   // 6
      drawPoint(Xend - radius + YCurrent, Ystart + radius - XCurrent, color, line_width);   // 7
      drawPoint(Xend - radius + YCurrent, Yend - radius + XCurrent, color, line_width);     // 0
    }
    if (Esp < 0)
      Esp += 4 * XCurrent + 6;
    else
    {
      Esp += 10 + 4 * (XCurrent - YCurrent);
      YCurrent--;
    }
    XCurrent++;
  }
}

void EPDDisplay::drawStar(uint16_t x_center, uint16_t y_center, uint16_t radius_outer, uint16_t radius_inner, uint8_t num_points, COLOR color, uint8_t line_width, DRAW_FILL draw_fill)
{
  if (x_center > width || y_center > height)
  {
    Debug("drawStar Input exceeds the normal display range\r\n");
    return;
  }

  // Ensure a minimum of 4 points for the star and maximum 10 points
  if (num_points < 3)
    num_points = 5;
  if (num_points > 10)
    num_points = 10;

  float angle_step = 2.0 * M_PI / (2 * num_points);
  float start_angle = -M_PI / 2;

  uint16_t points_x[20], points_y[20];
  uint8_t total_points = 2 * num_points;

  for (uint8_t i = 0; i < total_points; i++)
  {
    float angle = start_angle + i * angle_step;
    uint16_t radius = (i % 2 == 0) ? radius_outer : radius_inner;

    points_x[i] = x_center + (int16_t)(radius * cos(angle));
    points_y[i] = y_center + (int16_t)(radius * sin(angle));
  }

  if (draw_fill)
  {
    for (uint8_t i = 0; i < total_points; i++)
    {
      uint8_t next = (i + 1) % total_points;

      int16_t steps = max(abs((int16_t)points_x[next] - (int16_t)points_x[i]),
                          abs((int16_t)points_y[next] - (int16_t)points_y[i]));

      for (int16_t step = 0; step <= steps; step++)
      {
        uint16_t x = points_x[i] + ((points_x[next] - points_x[i]) * step) / steps;
        uint16_t y = points_y[i] + ((points_y[next] - points_y[i]) * step) / steps;
        drawLine(x_center, y_center, x, y, color, 1, LINE_SOLID);
      }
    }
  }
  else
  {
    for (uint8_t i = 0; i < total_points; i++)
    {
      uint8_t next = (i + 1) % total_points;
      drawLine(points_x[i], points_y[i], points_x[next], points_y[next], color, line_width, LINE_SOLID);
    }
  }
}

void EPDDisplay::drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, COLOR color, uint8_t line_width, DRAW_FILL draw_fill)
{
  if (x1 > width || y1 > height || x2 > width || y2 > height || x3 > width || y3 > height)
  {
    Debug("drawTriangle Input exceeds the normal display range\r\n");
    return;
  }

  if (draw_fill)
  {
    if (y1 > y2)
    {
      uint16_t temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }
    if (y2 > y3)
    {
      uint16_t temp = x2;
      x2 = x3;
      x3 = temp;
      temp = y2;
      y2 = y3;
      y3 = temp;
    }
    if (y1 > y2)
    {
      uint16_t temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }

    for (uint16_t y = y1; y <= y2; y++)
    {
      if (y2 != y1)
      {
        uint16_t xa = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
        uint16_t xb = x1 + ((x3 - x1) * (y - y1)) / (y3 - y1);
        if (xa > xb)
        {
          uint16_t temp = xa;
          xa = xb;
          xb = temp;
        }
        drawLine(xa, y, xb, y, color, line_width, LINE_SOLID);
      }
    }

    for (uint16_t y = y2 + 1; y <= y3; y++)
    {
      if (y3 != y2)
      {
        uint16_t xa = x2 + ((x3 - x2) * (y - y2)) / (y3 - y2);
        uint16_t xb = x1 + ((x3 - x1) * (y - y1)) / (y3 - y1);
        if (xa > xb)
        {
          uint16_t temp = xa;
          xa = xb;
          xb = temp;
        }
        drawLine(xa, y, xb, y, color, line_width, LINE_SOLID);
      }
    }
  }
  else
  {
    drawLine(x1, y1, x2, y2, color, line_width, LINE_SOLID);
    drawLine(x2, y2, x3, y3, color, line_width, LINE_SOLID);
    drawLine(x3, y3, x1, y1, color, line_width, LINE_SOLID);
  }
}

void EPDDisplay::drawEllipse(uint16_t x_center, uint16_t y_center, uint16_t radius_x, uint16_t radius_y, COLOR color, uint8_t line_width, DRAW_FILL draw_fill)
{
  if (x_center > width || y_center > height)
  {
    Debug("drawEllipse Input exceeds the normal display range\r\n");
    return;
  }

  int16_t x = 0;
  int16_t y = radius_y;
  int32_t rx2 = radius_x * radius_x;
  int32_t ry2 = radius_y * radius_y;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s = 2 * ry2 + rx2 * (1 - 2 * radius_y);

  if (draw_fill)
  {
    while (ry2 * x <= rx2 * y)
    {
      drawLine(x_center - x, y_center - y, x_center + x, y_center - y, color, line_width, LINE_SOLID);
      drawLine(x_center - x, y_center + y, x_center + x, y_center + y, color, line_width, LINE_SOLID);

      if (s >= 0)
      {
        s += fx2 * (1 - y);
        y--;
      }
      s += ry2 * ((4 * x) + 6);
      x++;
    }

    s = 2 * rx2 + ry2 * (1 - 2 * radius_x);
    while (y >= 0)
    {
      drawLine(x_center - x, y_center - y, x_center + x, y_center - y, color, line_width, LINE_SOLID);
      drawLine(x_center - x, y_center + y, x_center + x, y_center + y, color, line_width, LINE_SOLID);

      if (s <= 0)
      {
        s += fy2 * (1 + x);
        x++;
      }
      s += rx2 * (-4 * y + 6);
      y--;
    }
  }
  else
  {
    while (ry2 * x <= rx2 * y)
    {
      if (line_width > 1)
      {
        drawPoint(x_center + x, y_center + y, color, line_width);
        drawPoint(x_center - x, y_center + y, color, line_width);
        drawPoint(x_center + x, y_center - y, color, line_width);
        drawPoint(x_center - x, y_center - y, color, line_width);
      }
      else
      {
        drawPixel(x_center + x, y_center + y, color);
        drawPixel(x_center - x, y_center + y, color);
        drawPixel(x_center + x, y_center - y, color);
        drawPixel(x_center - x, y_center - y, color);
      }

      if (s >= 0)
      {
        s += fx2 * (1 - y);
        y--;
      }
      s += ry2 * ((4 * x) + 6);
      x++;
    }

    s = 2 * rx2 + ry2 * (1 - 2 * radius_x);
    while (y >= 0)
    {
      if (line_width > 1)
      {
        drawPoint(x_center + x, y_center + y, color, line_width);
        drawPoint(x_center - x, y_center + y, color, line_width);
        drawPoint(x_center + x, y_center - y, color, line_width);
        drawPoint(x_center - x, y_center - y, color, line_width);
      }
      else
      {
        drawPixel(x_center + x, y_center + y, color);
        drawPixel(x_center - x, y_center + y, color);
        drawPixel(x_center + x, y_center - y, color);
        drawPixel(x_center - x, y_center - y, color);
      }

      if (s <= 0)
      {
        s += fy2 * (1 + x);
        x++;
      }
      s += rx2 * (-4 * y + 6);
      y--;
    }
  }
}

void EPDDisplay::drawPolygon(const uint16_t *points_x, const uint16_t *points_y, uint8_t num_points, COLOR color, uint8_t line_width, DRAW_FILL draw_fill)
{
  if (num_points < 3)
  {
    Debug("drawPolygon needs at least 3 points\r\n");
    return;
  }

  if (draw_fill)
  {
    uint16_t min_y = points_y[0], max_y = points_y[0];

    for (uint8_t i = 1; i < num_points; i++)
    {
      if (points_y[i] < min_y)
        min_y = points_y[i];
      if (points_y[i] > max_y)
        max_y = points_y[i];
    }

    for (uint16_t y = min_y; y <= max_y; y++)
    {
      uint16_t intersections[20];
      uint8_t intersection_count = 0;

      for (uint8_t i = 0; i < num_points; i++)
      {
        uint8_t next = (i + 1) % num_points;

        if ((points_y[i] <= y && points_y[next] > y) ||
            (points_y[i] > y && points_y[next] <= y))
        {
          uint16_t x_intersect = points_x[i] +
                                 ((y - points_y[i]) * (points_x[next] - points_x[i])) /
                                     (points_y[next] - points_y[i]);

          if (intersection_count < 20)
          {
            intersections[intersection_count++] = x_intersect;
          }
        }
      }

      for (uint8_t i = 0; i < intersection_count - 1; i++)
      {
        for (uint8_t j = i + 1; j < intersection_count; j++)
        {
          if (intersections[i] > intersections[j])
          {
            uint16_t temp = intersections[i];
            intersections[i] = intersections[j];
            intersections[j] = temp;
          }
        }
      }

      for (uint8_t i = 0; i < intersection_count; i += 2)
      {
        if (i + 1 < intersection_count)
        {
          drawLine(intersections[i], y, intersections[i + 1], y, color, line_width, LINE_SOLID);
        }
      }
    }
  }
  else
  {
    for (uint8_t i = 0; i < num_points; i++)
    {
      uint8_t next = (i + 1) % num_points;
      drawLine(points_x[i], points_y[i], points_x[next], points_y[next], color, line_width, LINE_SOLID);
    }
  }
}