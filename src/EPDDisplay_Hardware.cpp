#include "EPDDisplay.h"

bool EPDDisplay::initialize()
{
    // Initialization of transformation variables
    rotate = EPDDisplay::ROTATE_0;
    mirror = EPDDisplay::MIRROR_NONE;

    if (isInitialized)
    {
        return true;
    }

    // Init buffers
    uint16_t imageSize = widthByte * heightByte;

    blackBuffer = (uint8_t *)malloc(imageSize);
    if (blackBuffer == NULL)
    {
        Debug("Failed to allocate memory for black buffer\r\n");
        return false;
    }

    redBuffer = (uint8_t *)malloc(imageSize);
    if (redBuffer == NULL)
    {
        free(blackBuffer);
        blackBuffer = NULL;
        Debug("Failed to allocate memory for black buffer\r\n");
        return false;
    }

    // init pins
    pinMode(m_BUSY_pin, INPUT);
    pinMode(m_RST_pin, OUTPUT);
    pinMode(m_DC_pin, OUTPUT);
    pinMode(m_CLK_pin, OUTPUT);
    pinMode(m_DIN_pin, OUTPUT);
    pinMode(m_CS_pin, OUTPUT);
    digitalWrite(m_CS_pin, HIGH);
    digitalWrite(m_CLK_pin, LOW);

    reset();

    // EPD hardware init start
    SendCommand(0x12); // SWRESET
    ReadBusy();        // waiting for the electronic paper IC to release the idle signal

    SendCommand(0x46); // Auto Write RAM
    SendData(0xF7);
    ReadBusy(); // waiting for the electronic paper IC to release the idle signal

    SendCommand(0x47); // Auto Write RAM
    SendData(0xF7);
    ReadBusy(); // waiting for the electronic paper IC to release the idle signal

    SendCommand(0x0C); // Soft start setting
    SendData(0xAE);
    SendData(0xC7);
    SendData(0xC3);
    SendData(0xC0);
    SendData(0x40);

    SendCommand(0x01); // Set MUX as 527
    SendData(0xAF);
    SendData(0x02);
    SendData(0x01);

    SendCommand(0x11); // Data entry mode
    SendData(0x01);

    SendCommand(0x44);
    SendData(0x00); // RAM x address start at 0
    SendData(0x00);
    SendData(0x6F); // RAM x address end at 36Fh -> 879
    SendData(0x03);
    SendCommand(0x45);
    SendData(0xAF); // RAM y address start at 20Fh;
    SendData(0x02);
    SendData(0x00); // RAM y address end at 00h;
    SendData(0x00);

    SendCommand(0x3C); // VBD
    SendData(0x01);    // LUT1, for white

    SendCommand(0x18);
    SendData(0X80);
    SendCommand(0x22);
    SendData(0XB1); // Load Temperature and waveform setting.
    SendCommand(0x20);
    ReadBusy(); // waiting for the electronic paper IC to release the idle signal

    SendCommand(0x4E);
    SendData(0x00);
    SendData(0x00);
    SendCommand(0x4F);
    SendData(0xAF);
    SendData(0x02);

    isInitialized = true;
    isSleep = false;
    return true;
}

void EPDDisplay::reset()
{
    digitalWrite(m_RST_pin, 0);
    delay(2);
    digitalWrite(m_RST_pin, 1);
    isSleep = false;
    delay(200);
}

void EPDDisplay::clear()
{
    if (!checkDisplayReady())
    {
        return;
    }

    ClearRed();
    ClearBlack();
    SendCommand(0x22);
    SendData(0xC7);
    SendCommand(0x20);
    delay(200);
    ReadBusy();
    Debug("clear EPD\r\n");
}

void EPDDisplay::display()
{
    if (!checkDisplayReady())
    {
        return;
    }

    uint32_t i, j;

    SendCommand(0x4F);
    SendData(0xAf);
    SendData(0x02);
    SendCommand(0x24); // BLACK
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(blackBuffer[i + j * widthByte]);
        }
    }
    ReadBusy();
    SendCommand(0x4F);
    SendData(0xAf);
    SendData(0x02);
    SendCommand(0x26); // RED
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(~redBuffer[i + j * widthByte]);
        }
    }
    SendCommand(0x22);
    SendData(0xC7);
    SendCommand(0x20);
    ReadBusy();
    Debug("display\r\n");
}

void EPDDisplay::sleep()
{
    if (!isInitialized || isSleep)
    {
        Debug("EPD not initialized or already in sleep mode\r\n");
        return;
    }

    SendCommand(0x10);
    SendData(0x01);
    delay(100);
    isSleep = true;
    Debug("e-Paper enters sleep\r\n");
}

bool EPDDisplay::isInSleep()
{
    return isSleep;
}

void EPDDisplay::wakeUp()
{
    if (isSleep)
    {
        Debug("Waking up e-Paper from sleep mode\r\n");
        reset();
        Debug("e-Paper wake up complete\r\n");
    }
    else
    {
        Debug("e-Paper is already awake\r\n");
    }
}

/****************************
 * PRIVATE FUNCTIONS
 ****************************/

bool EPDDisplay::checkDisplayReady()
{
    if (!isInitialized)
    {
        Debug("EPD not initialized\r\n");
        return false;
    }

    if (isSleep)
    {
        Debug("EPD is in sleep mode\r\n");
        return false;
    }

    return true;
}

void EPDDisplay::ClearRed()
{
    uint32_t i, j;

    ReadBusy();
    SendCommand(0x4F);
    SendData(0xAf);
    SendData(0x02);
    SendCommand(0x26); // RED
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(0X00);
        }
    }
}

void EPDDisplay::ClearBlack()
{
    uint32_t i, j;

    ReadBusy();
    SendCommand(0x4F);
    SendData(0xAf);
    SendData(0x02);
    SendCommand(0x24); // BLOCK
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(0XFF);
        }
    }
}

void EPDDisplay::SendCommand(uint8_t Reg)
{
    digitalWrite(m_DC_pin, 0);
    digitalWrite(m_CS_pin, 0);
    SPI_WriteByte(Reg);
    digitalWrite(m_CS_pin, 1);
}

void EPDDisplay::SendData(uint8_t Data)
{
    digitalWrite(m_DC_pin, 1);
    digitalWrite(m_CS_pin, 0);
    SPI_WriteByte(Data);
    digitalWrite(m_CS_pin, 1);
}

void EPDDisplay::ReadBusy()
{
    uint8_t busy;
    Debug("e-Paper busy\r\n");
    do
    {
        busy = digitalRead(m_BUSY_pin);
    } while (busy);
    Debug("e-Paper busy release\r\n");
    delay(200);
}

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

void EPDDisplay::SPI_WriteByte(uint8_t data)
{
    digitalWrite(m_CS_pin, GPIO_PIN_RESET);

    for (int i = 0; i < 8; i++)
    {
        if ((data & 0x80) == 0)
            digitalWrite(m_DIN_pin, GPIO_PIN_RESET);
        else
            digitalWrite(m_DIN_pin, GPIO_PIN_SET);

        data <<= 1;
        digitalWrite(m_CLK_pin, GPIO_PIN_SET);
        digitalWrite(m_CLK_pin, GPIO_PIN_RESET);
    }
    digitalWrite(m_CS_pin, GPIO_PIN_SET);
}
