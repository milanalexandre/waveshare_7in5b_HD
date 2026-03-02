#include "EPDDisplay.h"

// Extended character tables (defined in their respective font files)
extern const uint8_t fontExt8_Table[];
extern const uint8_t fontExt12_Table[];
extern const uint8_t fontExt16_Table[];
extern const uint8_t fontExt20_Table[];
extern const uint8_t fontExt24_Table[];

// Sorted Unicode codepoints for binary search
static const uint32_t fontExtCodepoints[45] = {
    0x00A1U, 0x00B0U, 0x00B1U, 0x00BFU, 0x00C0U, 0x00C1U, 0x00C2U, 0x00C4U,
    0x00C7U, 0x00C8U, 0x00C9U, 0x00CAU, 0x00CBU, 0x00CDU, 0x00CEU, 0x00CFU,
    0x00D1U, 0x00D3U, 0x00D6U, 0x00D9U, 0x00DAU, 0x00DBU, 0x00DCU, 0x00DFU,
    0x00E0U, 0x00E1U, 0x00E2U, 0x00E4U, 0x00E7U, 0x00E8U, 0x00E9U, 0x00EAU,
    0x00EBU, 0x00EDU, 0x00EEU, 0x00EFU, 0x00F1U, 0x00F3U, 0x00F6U, 0x00F9U,
    0x00FAU, 0x00FBU, 0x00FCU, 0x00FFU, 0x20ACU
};

static int16_t fontExtLookup(uint32_t codepoint)
{
    int16_t lo = 0, hi = 44;
    while (lo <= hi) {
        int16_t mid = (lo + hi) >> 1;
        if (fontExtCodepoints[mid] == codepoint) return mid;
        if (fontExtCodepoints[mid] < codepoint) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

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

    const uint8_t *s = (const uint8_t *)pString;
    while (*s != '\0')
    {
        // Decode one UTF-8 codepoint
        uint32_t cp;
        if (*s < 0x80)
        {
            cp = *s++;
        }
        else if ((*s & 0xE0) == 0xC0 && s[1] != 0)
        {
            cp = ((uint32_t)(s[0] & 0x1F) << 6) | (s[1] & 0x3F);
            s += 2;
        }
        else if ((*s & 0xF0) == 0xE0 && s[1] != 0 && s[2] != 0)
        {
            cp = ((uint32_t)(s[0] & 0x0F) << 12) | ((uint32_t)(s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            s += 3;
        }
        else
        {
            s++; // skip invalid / unsupported byte
            continue;
        }

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

        drawCodepoint(Xpoint, Ypoint, cp, Font, color_foreground, color_background);
        Xpoint += Font->width;
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void EPDDisplay::drawCharBitmap(uint16_t Xpoint, uint16_t Ypoint, const uint8_t *ptr, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    for (uint16_t Page = 0; Page < Font->height; Page++)
    {
        for (uint16_t Column = 0; Column < Font->width; Column++)
        {
            if (EPDDisplay::NULL_COLOR == color_background)
            {
                if (*ptr & (0x80 >> (Column % 8)))
                    drawPixel(Xpoint + Column, Ypoint + Page, color_foreground);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8)))
                    drawPixel(Xpoint + Column, Ypoint + Page, color_foreground);
                else
                    drawPixel(Xpoint + Column, Ypoint + Page, color_background);
            }
            if (Column % 8 == 7)
                ptr++;
        }
        if (Font->width % 8 != 0)
            ptr++;
    }
}

const uint8_t *EPDDisplay::getExtTable(sFONT *Font)
{
    if (Font == &Font8)  return fontExt8_Table;
    if (Font == &Font12) return fontExt12_Table;
    if (Font == &Font16) return fontExt16_Table;
    if (Font == &Font20) return fontExt20_Table;
    if (Font == &Font24) return fontExt24_Table;
    return nullptr;
}

void EPDDisplay::drawCodepoint(uint16_t Xpoint, uint16_t Ypoint, uint32_t codepoint, sFONT *Font, COLOR color_foreground, COLOR color_background)
{
    const uint16_t bytes_per_char = Font->height * (Font->width / 8 + (Font->width % 8 ? 1 : 0));
    const uint8_t *ptr = nullptr;

    if (codepoint >= 0x20 && codepoint <= 0x7E)
    {
        // Standard ASCII range
        ptr = &Font->table[(codepoint - ' ') * bytes_per_char];
    }
    else
    {
        // Look up in extended table
        int16_t idx = fontExtLookup(codepoint);
        if (idx >= 0)
        {
            const uint8_t *extTable = getExtTable(Font);
            if (extTable != nullptr)
                ptr = &extTable[(uint16_t)idx * bytes_per_char];
        }
    }

    if (ptr == nullptr)
    {
        // Unsupported codepoint — fall back to '?'
        ptr = &Font->table[('?' - ' ') * bytes_per_char];
    }

    drawCharBitmap(Xpoint, Ypoint, ptr, Font, color_foreground, color_background);
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