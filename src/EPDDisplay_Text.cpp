#include "EPDDisplay.h"

void EPDDisplay::drawChar(uint16_t Xpoint, uint16_t Ypoint, const char Acsii_Char, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    uint16_t Page, Column;

    if (Xpoint > width || Ypoint > height)
    {
        Debug("Paint_DrawChar Input exceeds the normal display range\r\n");
        return;
    }

    // Get the offset address of the character in the font table
    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->height * (Font->width / 8 + (Font->width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];

    for (Page = 0; Page < Font->height; Page++)
    {
        for (Column = 0; Column < Font->width; Column++)
        {
            // To determine whether the font background color and screen background color is consistent
            if (EPDDisplay::NULL_COLOR == color_background)
            { // this process is to speed up the scan
                if (*ptr & (0x80 >> (Column % 8)))
                    drawPixel(Xpoint + Column, Ypoint + Page, color_foreground);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8)))
                {
                    drawPixel(Xpoint + Column, Ypoint + Page, color_foreground);
                }
                else
                {
                    drawPixel(Xpoint + Column, Ypoint + Page, color_background);
                }
            }
            // One pixel is 8 bits
            if (Column % 8 == 7)
                ptr++;
        }
        if (Font->width % 8 != 0)
            ptr++;
    }
}

void EPDDisplay::drawString(uint16_t Xstart, uint16_t Ystart, const char *pString, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    uint16_t Xpoint = Xstart;
    uint16_t Ypoint = Ystart;

    if (Xstart > width || Ystart > height)
    {
        Debug("drawString Input exceeds the normal display range\r\n");
        return;
    }

    while (*pString != '\0')
    {
        if ((Xpoint + Font->width) > width)
        {
            Xpoint = Xstart;
            Ypoint += Font->height;
        }

        if ((Ypoint + Font->height) > height)
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        drawChar(Xpoint, Ypoint, *pString, Font, color_foreground, color_background);

        pString++;
        Xpoint += Font->width;
    }
}

void EPDDisplay::drawNumber(uint16_t Xpoint, uint16_t Ypoint, int32_t number, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    if (Xpoint > width || Ypoint > height)
    {
        Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[255] = {0}, Num_Array[255] = {0};
    uint8_t *pStr = Str_Array;

    bool negative = false;
    // Converts a number to a string
    // negative
    if (number < 0)
    {
        negative = true;
        number = -number;
    }
    // number == 0
    Num_Array[Num_Bit] = number % 10 + '0';
    Num_Bit++;
    number /= 10;

    while (number)
    {
        Num_Array[Num_Bit] = number % 10 + '0';
        Num_Bit++;
        number /= 10;
    }

    if (negative)
    {
        Num_Array[Num_Bit] = '-';
        Num_Bit++;
    }

    // The string is inverted
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // show
    drawString(Xpoint, Ypoint, (const char *)pStr, Font, color_foreground, color_background);
}

void EPDDisplay::drawFloat(uint16_t Xpoint, uint16_t Ypoint, float Number, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    if (Xpoint > width || Ypoint > height)
    {
        Debug("Paint_DrawFloat Input exceeds the normal display range\r\n");
        return;
    }

    int16_t Str_Bit = 0;
    uint8_t Str_Array[255] = {0};
    uint8_t *pStr = Str_Array;

    // Converts a float number to a string
    sprintf((char *)Str_Array, "%.2f", Number);

    // Show
    drawString(Xpoint, Ypoint, (const char *)pStr, Font, color_foreground, color_background);
}

void EPDDisplay::drawTime(uint16_t Xstart, uint16_t Ystart, uint8_t hour, uint8_t minute, uint8_t second, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint16_t Dx = Font->width;

    drawChar(Xstart, Ystart, value[hour / 10], Font, color_foreground, color_background);
    drawChar(Xstart + Dx, Ystart, value[hour % 10], Font, color_foreground, color_background);
    drawChar(Xstart + Dx + Dx / 4 + Dx / 2, Ystart, ':', Font, color_foreground, color_background);
    drawChar(Xstart + Dx * 2 + Dx / 2, Ystart, value[minute / 10], Font, color_foreground, color_background);
    drawChar(Xstart + Dx * 3 + Dx / 2, Ystart, value[minute % 10], Font, color_foreground, color_background);
    drawChar(Xstart + Dx * 4 + Dx / 2 - Dx / 4, Ystart, ':', Font, color_foreground, color_background);
    drawChar(Xstart + Dx * 5, Ystart, value[second / 10], Font, color_foreground, color_background);
    drawChar(Xstart + Dx * 6, Ystart, value[second % 10], Font, color_foreground, color_background);
}