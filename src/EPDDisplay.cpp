#include "EPDDisplay.h"

// Constructor with pin parameters
EPDDisplay::EPDDisplay(
    int busy_pin,
    int rst_pin,
    int dc_pin,
    int cs_pin,
    int clk_pin,
    int din_pin) : blackBuffer(NULL),
                   redBuffer(NULL),
                   isInitialized(false),
                   isSleep(false),
                   width(EPD_7IN5B_HD_WIDTH),
                   height(EPD_7IN5B_HD_HEIGHT),
                   widthByte((EPD_7IN5B_HD_WIDTH % 8 == 0) ? (EPD_7IN5B_HD_WIDTH / 8) : (EPD_7IN5B_HD_WIDTH / 8 + 1)),
                   heightByte(EPD_7IN5B_HD_HEIGHT),
                   widthMemory(EPD_7IN5B_HD_WIDTH),   // 880
                   heightMemory(EPD_7IN5B_HD_HEIGHT), // 528
                   rotate(EPDDisplay::ROTATE_0),
                   mirror(EPDDisplay::MIRROR_NONE),
                   m_BUSY_pin(busy_pin),
                   m_RST_pin(rst_pin),
                   m_DC_pin(dc_pin),
                   m_CS_pin(cs_pin),
                   m_CLK_pin(clk_pin),
                   m_DIN_pin(din_pin)
{
}

// Destructor
EPDDisplay::~EPDDisplay()
{
    if (blackBuffer != NULL)
    {
        free(blackBuffer);
        blackBuffer = NULL;
    }

    if (redBuffer != NULL)
    {
        free(redBuffer);
        redBuffer = NULL;
    }
}
