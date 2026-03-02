/**
 * @file EPDDisplay_Hardware.cpp
 * @brief Hardware-level operations: initialization, SPI communication,
 *        display refresh, clear, sleep, and wakeup.
 *
 * SPI is bit-banged (software SPI): each bit is clocked manually via GPIO.
 * This avoids conflicts with the ESP32 hardware SPI peripheral and lets any
 * GPIO be used for any signal. The trade-off is slower bit rate (~1 MHz
 * effective), but the bottleneck is always the display's own processing time
 * (~15–20 s for a full tricolor refresh), not the SPI transfer speed.
 *
 * Buffer encoding convention (stored internally):
 *   - blackBuffer  bit=1 → white or red pixel;  bit=0 → black pixel
 *   - redBuffer    bit=1 → white or black pixel; bit=0 → red pixel
 *
 * When sending to the display:
 *   - Command 0x24 (BW plane): blackBuffer is sent as-is
 *   - Command 0x26 (Red plane): ~redBuffer is sent (bitwise NOT), because
 *     the controller expects bit=1 to mean "red active"
 */
#include "EPDDisplay.h"

// ── Private: sends the full controller init sequence ─────────────────────────
// Separated from initialize() so that wakeUp() can re-apply all settings
// after a hardware reset without re-allocating buffers.
void EPDDisplay::hwInit()
{
    SendCommand(0x12); // Software Reset (SWRESET) — restores all registers to defaults
    ReadBusy();        // Wait until the controller finishes its internal reset

    // Auto Write RAM: pre-fills both BW and Red RAM with 0xF7 (all-white pattern)
    // This ensures a clean state before the first real frame is sent.
    SendCommand(0x46); // Auto Write BW RAM
    SendData(0xF7);
    ReadBusy();

    SendCommand(0x47); // Auto Write Red RAM
    SendData(0xF7);
    ReadBusy();

    // Booster Soft Start (0x0C): configures the charge pump phases A/B/C/D.
    SendCommand(0x0C);
    SendData(0xAE); // Phase A
    SendData(0xC7); // Phase B
    SendData(0xC3); // Phase C
    SendData(0xC0); // Phase D
    SendData(0x40);

    // Driver Output Control (0x01): sets MUX = 0x02AF → 528 gate lines (0-indexed 0..527)
    SendCommand(0x01);
    SendData(0xAF); // MUX low byte
    SendData(0x02); // MUX high byte
    SendData(0x01); // Gate scanning: top-to-bottom

    // Data Entry Mode (0x11): X-increment then Y-increment (left-to-right, top-to-bottom)
    SendCommand(0x11);
    SendData(0x01);

    // Set RAM X address window: columns 0x0000 to 0x036F (0–879)
    SendCommand(0x44);
    SendData(0x00); // X start low byte
    SendData(0x00); // X start high byte
    SendData(0x6F); // X end low byte  (0x036F = 879)
    SendData(0x03); // X end high byte

    // Set RAM Y address window: rows 0x02AF down to 0x0000 (527–0)
    SendCommand(0x45);
    SendData(0xAF); // Y start low byte  (0x02AF = 527)
    SendData(0x02); // Y start high byte
    SendData(0x00); // Y end low byte    (0)
    SendData(0x00); // Y end high byte

    // Border Waveform Control: border pixel = white (LUT1)
    SendCommand(0x3C);
    SendData(0x01);

    // Temperature sensor: use internal sensor
    SendCommand(0x18);
    SendData(0x80);
    // Load temperature + waveform LUT, then activate
    SendCommand(0x22);
    SendData(0xB1);
    SendCommand(0x20);
    ReadBusy(); // Wait for LUT load to complete

    // Set initial RAM address counters to top-left of window
    SendCommand(0x4E); // X address counter = 0
    SendData(0x00);
    SendData(0x00);
    SendCommand(0x4F); // Y address counter = 527 (top of display)
    SendData(0xAF);
    SendData(0x02);
}

bool EPDDisplay::initialize()
{
    // Reset transform state on every call
    rotate = EPDDisplay::ROTATE_0;
    mirror = EPDDisplay::MIRROR_NONE;

    if (isInitialized)
    {
        return true;
    }

    // Allocate framebuffers — use uint32_t to avoid uint8_t × uint16_t truncation
    uint32_t imageSize = (uint32_t)widthByte * heightByte;

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
        Debug("Failed to allocate memory for red buffer\r\n");
        return false;
    }

    // Configure GPIO pins
    pinMode(m_BUSY_pin, INPUT);
    pinMode(m_RST_pin, OUTPUT);
    pinMode(m_DC_pin, OUTPUT);
    pinMode(m_CLK_pin, OUTPUT);
    pinMode(m_DIN_pin, OUTPUT);
    pinMode(m_CS_pin, OUTPUT);
    digitalWrite(m_CS_pin, HIGH);
    digitalWrite(m_CLK_pin, LOW);

    reset();
    hwInit();

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

    uint32_t imageSize = (uint32_t)widthByte * heightByte;
    memset(blackBuffer, 0xFF, imageSize);
    memset(redBuffer, 0xFF, imageSize);

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

    // ── Send Black/White plane (command 0x24) ──────────────────────────────
    // Reset the Y address counter to start from the top row (row 527)
    SendCommand(0x4F);
    SendData(0xAF);
    SendData(0x02);
    SendCommand(0x24); // Write to BW RAM
    // Send all 110 × 528 = 58,080 bytes of the black buffer.
    // blackBuffer encoding: bit=1 → white pixel, bit=0 → black pixel (controller native).
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(blackBuffer[i + j * widthByte]);
        }
    }
    ReadBusy(); // Wait for BW RAM write to complete

    // ── Send Red plane (command 0x26) ──────────────────────────────────────
    SendCommand(0x4F); // Reset Y address counter again
    SendData(0xAF);
    SendData(0x02);
    SendCommand(0x26); // Write to Red RAM
    // redBuffer encoding: bit=0 → red pixel, bit=1 → no red.
    // The controller expects bit=1 for "red active", so we invert with ~.
    for (j = 0; j < heightByte; j++)
    {
        for (i = 0; i < widthByte; i++)
        {
            SendData(~redBuffer[i + j * widthByte]);
        }
    }

    // ── Trigger full panel refresh ─────────────────────────────────────────
    // 0x22 with 0xC7: display update sequence = Load waveform + enable clock +
    // enable analog + display sequence + disable analog + disable clock.
    SendCommand(0x22);
    SendData(0xC7);
    SendCommand(0x20); // Master activation — begins the ~15–20 s e-paper refresh
    ReadBusy();        // Block until the panel refresh is complete
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
        hwInit();
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
    SendCommand(0x24);
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
    // The BUSY pin is LOW while the controller is processing and HIGH when idle.
    // Poll until it goes HIGH, with a 30 s watchdog to avoid hanging on hardware fault.
    uint32_t start = millis();
    do
    {
        busy = digitalRead(m_BUSY_pin);
        if (millis() - start > 30000)
        {
            Debug("e-Paper BUSY timeout!\r\n");
            break;
        }
    } while (busy);
    Debug("e-Paper busy release\r\n");
    delay(200); // Additional settling time after BUSY clears
}

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

/**
 * Bit-banged SPI write: transmits one byte MSB-first using Mode 0
 * (CPOL=0, CPHA=0 — data sampled on rising clock edge).
 *
 * Timing per bit:
 *   1. Set DIN to the bit value (MSB first)
 *   2. Pulse CLK HIGH  → controller latches DIN
 *   3. Pull CLK LOW
 * CS is asserted (LOW) by the caller (SendCommand / SendData) before this
 * function is called, but this function also re-asserts CS around the byte
 * to guard against re-entrant calls.
 */
void EPDDisplay::SPI_WriteByte(uint8_t data)
{
    digitalWrite(m_CS_pin, GPIO_PIN_RESET);

    for (int i = 0; i < 8; i++)
    {
        // Output the MSB of `data` onto DIN
        if ((data & 0x80) == 0)
            digitalWrite(m_DIN_pin, GPIO_PIN_RESET);
        else
            digitalWrite(m_DIN_pin, GPIO_PIN_SET);

        data <<= 1; // Shift next bit into MSB position

        // Rising edge: controller latches DIN
        digitalWrite(m_CLK_pin, GPIO_PIN_SET);
        // Falling edge: prepare for next bit
        digitalWrite(m_CLK_pin, GPIO_PIN_RESET);
    }
    digitalWrite(m_CS_pin, GPIO_PIN_SET);
}
