#include "EPDDisplay.h"

void EPDDisplay::drawPixel(uint16_t x, uint16_t y, COLOR color)
{
    if (x > width || y > height)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }
    uint16_t X, Y;
    switch (rotate)
    {
    case EPDDisplay::ROTATE_0:
        X = x;
        Y = y;
        break;
    case EPDDisplay::ROTATE_90:
        X = widthMemory - y - 1;
        Y = x;
        break;
    case EPDDisplay::ROTATE_180:
        X = widthMemory - x - 1;
        Y = heightMemory - y - 1;
        break;
    case EPDDisplay::ROTATE_270:
        X = y;
        Y = heightMemory - x - 1;
        break;
    default:
        Debug("Invalid rotation mode\r\n");
        return;
    }

    switch (mirror)
    {
    case EPDDisplay::MIRROR_NONE:
        break;
    case EPDDisplay::MIRROR_HORIZONTAL:
        X = widthMemory - X - 1;
        break;
    case EPDDisplay::MIRROR_VERTICAL:
        Y = heightMemory - Y - 1;
        break;
    case EPDDisplay::MIRROR_ORIGIN:
        X = widthMemory - X - 1;
        Y = heightMemory - Y - 1;
        break;
    default:
        Debug("Invalid mirror mode\r\n");
        return;
    }

    if (X > widthMemory || Y > heightMemory)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }

    uint32_t Addr = X / 8 + Y * widthByte;

    switch (color)
    {
    case EPDDisplay::BLACK:
        blackBuffer[Addr] = blackBuffer[Addr] & ~(0x80 >> (X % 8));
        redBuffer[Addr] = redBuffer[Addr] | (0x80 >> (X % 8));
        break;
    case EPDDisplay::RED:
        blackBuffer[Addr] = blackBuffer[Addr] | (0x80 >> (X % 8));
        redBuffer[Addr] = redBuffer[Addr] & ~(0x80 >> (X % 8));
        break;
    case EPDDisplay::WHITE:
        blackBuffer[Addr] = blackBuffer[Addr] | (0x80 >> (X % 8));
        redBuffer[Addr] = redBuffer[Addr] | (0x80 >> (X % 8));
        break;
    case EPDDisplay::NULL_COLOR:
        break;
    }
}

void EPDDisplay::fillScreen(COLOR color)
{
    for (uint16_t Y = 0; Y < heightByte; Y++)
    {
        for (uint16_t X = 0; X < widthByte; X++)
        { // 8 pixel =  1 byte
            uint32_t Addr = X + Y * widthByte;
            switch (color)
            {
            case EPDDisplay::BLACK:
                blackBuffer[Addr] = 0x00;
                redBuffer[Addr] = 0xFF;
                break;
            case EPDDisplay::WHITE:
                blackBuffer[Addr] = 0xFF;
                redBuffer[Addr] = 0xFF;
                break;
            case EPDDisplay::RED:
                blackBuffer[Addr] = 0xFF;
                redBuffer[Addr] = 0x00;
                break;
            case EPDDisplay::NULL_COLOR:
                break;
            }
        }
    }
}

void EPDDisplay::setRotation(uint8_t rotate)
{
    if (
        rotate == EPDDisplay::ROTATE_0 || rotate == EPDDisplay::ROTATE_90 || rotate == EPDDisplay::ROTATE_180 || rotate == EPDDisplay::ROTATE_270)
    {
        rotate = rotate;
    }
    else
    {
        Debug("rotate should be EPDDisplay::ROTATE_0, EPDDisplay::ROTATE_90, EPDDisplay::ROTATE_180, EPDDisplay::ROTATE_270\r\n");
    }
}

void EPDDisplay::setMirror(uint8_t mirror)
{
    if (mirror == EPDDisplay::MIRROR_NONE || mirror == EPDDisplay::MIRROR_HORIZONTAL ||
        mirror == EPDDisplay::MIRROR_VERTICAL || mirror == EPDDisplay::MIRROR_ORIGIN)
    {
        mirror = mirror;
    }
    else
    {
        Debug("mirror should be EPDDisplay::MIRROR_NONE, EPDDisplay::MIRROR_HORIZONTAL, EPDDisplay::MIRROR_VERTICAL, EPDDisplay::MIRROR_ORIGIN\r\n");
    }
}

void EPDDisplay::drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap)
{
    this->drawBitmap(x, y, width, height, bitmap, EPDDisplay::BLACK, EPDDisplay::WHITE);
}

void EPDDisplay::drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap, COLOR active_color, COLOR inactive_color)
{
    if ((x >= this->width) || (y >= this->height))
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }
    uint16_t X, Y;
    for (Y = 0; Y < height; Y++)
    {
        for (X = 0; X < width; X++)
        {
            // Check if we're still within display bounds
            if ((x + X) >= this->width || (y + Y) >= this->height)
                continue;

            if (bitmap[(X + Y * width) / 8] & (0x80 >> (X % 8)))
            {
                drawPixel(x + X, y + Y, active_color);
            }
            else
            {
                drawPixel(x + X, y + Y, inactive_color);
            }
        }
    }
}
