/**
 * @file EPDDisplay_Shapes.cpp
 * @brief Simple shape primitives: circle, rectangle, line, point.
 *
 * Drawing algorithms used:
 *
 * drawCircle — Bresenham midpoint circle algorithm.
 *   Starts at (0, R) and uses an error accumulator (Esp) to decide whether
 *   to decrement Y on each step. Plots 8 symmetric octant points per
 *   iteration, giving O(R) time complexity with integer arithmetic only.
 *
 * drawLine — Bresenham line algorithm.
 *   Uses cumulative error (Esp = dx + dy) to step along the major axis and
 *   conditionally step the minor axis when error exceeds the threshold.
 *   Supports dotted style by counting drawn vs. skipped pixels.
 *
 * drawRectangle — Four drawLine calls (outline) or horizontal scan-fill.
 *
 * drawPoint — Square block of (2*width-1)² pixels centered on (x,y).
 */
#include "EPDDisplay.h"

void EPDDisplay::drawCircle(uint16_t Xcenter, uint16_t Ycenter, uint16_t radius, COLOR color, uint8_t line_width, DRAW_FILL draw_fill)
{
    if (Xcenter > width || Ycenter > height)
    {
        Debug("Paint_DrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    // Bresenham midpoint circle: start at top (0, R), iterate to the 45° point.
    // Esp is the decision variable: Esp = 3 - 2*R initially.
    //   Esp < 0: move to (x+1, y)     → Esp += 4*x + 6
    //   Esp ≥ 0: move to (x+1, y-1)  → Esp += 4*(x-y) + 10, then y--
    int16_t Xcurrent, Ycurrent;
    Xcurrent = 0;
    Ycurrent = radius;

    int16_t Esp = 3 - (radius << 1); // 3 - 2*R

    int16_t sCountY;
    if (draw_fill)
    {
        while (Xcurrent <= Ycurrent)
        { // Realistic circles
            for (sCountY = Xcurrent; sCountY <= Ycurrent; sCountY++)
            {
                drawPoint(Xcenter + Xcurrent, Ycenter + sCountY, color, line_width); // 1
                drawPoint(Xcenter - Xcurrent, Ycenter + sCountY, color, line_width); // 2
                drawPoint(Xcenter - sCountY, Ycenter + Xcurrent, color, line_width); // 3
                drawPoint(Xcenter - sCountY, Ycenter - Xcurrent, color, line_width); // 4
                drawPoint(Xcenter - Xcurrent, Ycenter - sCountY, color, line_width); // 5
                drawPoint(Xcenter + Xcurrent, Ycenter - sCountY, color, line_width); // 6
                drawPoint(Xcenter + sCountY, Ycenter - Xcurrent, color, line_width); // 7
                drawPoint(Xcenter + sCountY, Ycenter + Xcurrent, color, line_width); // 0
            }
            if (Esp < 0)
                Esp += 4 * Xcurrent + 6;
            else
            {
                Esp += 10 + 4 * (Xcurrent - Ycurrent);
                Ycurrent--;
            }
            Xcurrent++;
        }
    }
    else
    {
        while (Xcurrent <= Ycurrent)
        {
            drawPoint(Xcenter + Xcurrent, Ycenter + Ycurrent, color, line_width); // 1
            drawPoint(Xcenter - Xcurrent, Ycenter + Ycurrent, color, line_width); // 2
            drawPoint(Xcenter - Ycurrent, Ycenter + Xcurrent, color, line_width); // 3
            drawPoint(Xcenter - Ycurrent, Ycenter - Xcurrent, color, line_width); // 4
            drawPoint(Xcenter - Xcurrent, Ycenter - Ycurrent, color, line_width); // 5
            drawPoint(Xcenter + Xcurrent, Ycenter - Ycurrent, color, line_width); // 6
            drawPoint(Xcenter + Ycurrent, Ycenter - Xcurrent, color, line_width); // 7
            drawPoint(Xcenter + Ycurrent, Ycenter + Xcurrent, color, line_width); // 0

            if (Esp < 0)
                Esp += 4 * Xcurrent + 6;
            else
            {
                Esp += 10 + 4 * (Xcurrent - Ycurrent);
                Ycurrent--;
            }
            Xcurrent++;
        }
    }
}

void EPDDisplay::drawRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, COLOR color, uint8_t line_width, LINE_STYLE line_style, DRAW_FILL draw_Fill)
{
    if (draw_Fill)
    {
        uint16_t Ypoint;
        for (Ypoint = Ystart; Ypoint < Yend; Ypoint++)
        {
            drawLine(Xstart, Ypoint, Xend, Ypoint, color, line_width, EPDDisplay::LINE_SOLID);
        }
    }
    else
    {
        drawLine(Xstart, Ystart, Xend, Ystart, color, line_width, line_style);
        drawLine(Xstart, Ystart, Xstart, Yend, color, line_width, line_style);
        drawLine(Xend, Yend, Xend, Ystart, color, line_width, line_style);
        drawLine(Xend, Yend, Xstart, Yend, color, line_width, line_style);
    }
}

void EPDDisplay::drawLine(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, COLOR color, uint8_t line_width, LINE_STYLE line_style)
{
    if (Xstart > width || Ystart > height ||
        Xend > width || Yend > height || line_width == 0)
    {
        Debug("drawLine Input exceeds the normal display range\r\n");
        return;
    }

    uint16_t Xpoint = Xstart;
    uint16_t Ypoint = Ystart;
    int dx = abs((int)Xend - (int)Xstart);
    int dy = -abs((int)Yend - (int)Ystart);

    // Step directions: +1 toward end, -1 away from end
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    // Bresenham error accumulator.
    // dx is always positive; dy is always ≤ 0 (negated above).
    // When 2*Esp ≥ dy: advance X (major axis step).
    // When 2*Esp ≤ dx: advance Y (minor axis step).
    int Esp = dx + dy;
    uint16_t Dotted_Len = 0;

    for (;;)
    {
        // Painted dotted line, 2 point is really virtual
        if (line_style == EPDDisplay::LINE_DOTTED)
        {
            Dotted_Len++;
            if (Dotted_Len < (line_width * 3))
            {
                if (line_width > 1)
                {
                    drawPoint(Xpoint, Ypoint, color, line_width);
                }
                else
                {
                    drawPixel(Xpoint, Ypoint, color);
                }
            }
            else if (Dotted_Len <= (line_width * 4))
            {
            }
            else
            {
                Dotted_Len = 0;
            }
        }
        else
        {
            if (line_width > 1)
            {
                drawPoint(Xpoint, Ypoint, color, line_width);
            }
            else
            {
                drawPixel(Xpoint, Ypoint, color);
            }
        }
        if (2 * Esp >= dy)
        {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx)
        {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void EPDDisplay::drawPoint(uint16_t Xpoint, uint16_t Ypoint, COLOR color, uint8_t point_width)
{
    if ((Xpoint + point_width - 1) > width || (Ypoint + point_width - 1) > height)
    {
        Debug("drawPoint Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XDir_Num, YDir_Num;
    int16_t offset = (int16_t)(point_width - 1);

    for (XDir_Num = 0; XDir_Num < 2 * point_width - 1; XDir_Num++)
    {
        if ((int16_t)Xpoint + XDir_Num - offset < 0)
            continue;
        for (YDir_Num = 0; YDir_Num < 2 * point_width - 1; YDir_Num++)
        {
            if ((int16_t)Ypoint + YDir_Num - offset < 0)
                continue;
            drawPixel(Xpoint + XDir_Num - offset, Ypoint + YDir_Num - offset, color);
        }
    }
}
