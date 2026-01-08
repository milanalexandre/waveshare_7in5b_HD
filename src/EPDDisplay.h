#ifndef __EPDDISPLAY_H
#define __EPDDISPLAY_H
#include <Arduino.h>

// Definitions for the EPD 7.5" B HD display
#define EPD_7IN5B_HD_WIDTH 880
#define EPD_7IN5B_HD_HEIGHT 528

#ifdef DEBUG
#define Debug(__info) Serial.print(__info)
#else
#define Debug(__info)
#endif

/**
 * @brief Class to manage display on a 7.5" B HD e-Paper screen with black, white and red colors
 * This class encapsulates the functionalities of Paint and EPD_7IN5B_HD
 */
class EPDDisplay
{
public:
    /**************
     * Types
     **************/

    /**
     * @brief Font structure definition
     * Contains font data table and dimensions
     */
    typedef struct _tFont
    {
        const uint8_t *table;
        uint16_t width;
        uint16_t height;
    } sFONT;

    /**************
     * Variables
     **************/
    /**
     * @brief Color enumeration for display
     * Available colors: NULL_COLOR, WHITE, BLACK, RED
     */
    typedef enum
    {
        NULL_COLOR = 0, // Useful for not putting a background for text writing
        WHITE = 1,
        BLACK = 2,
        RED = 3
    } COLOR;

    /**
     * @brief Rotation enumeration for display orientation
     * Available rotations: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
     */
    typedef enum
    {
        ROTATE_0 = 1,
        ROTATE_90 = 2,
        ROTATE_180 = 3,
        ROTATE_270 = 4
    } ROTATE;

    /**
     * @brief Mirror enumeration for display mirroring
     * Available options: MIRROR_NONE, MIRROR_HORIZONTAL, MIRROR_VERTICAL, MIRROR_ORIGIN
     */
    typedef enum
    {
        MIRROR_NONE = 0x00,
        MIRROR_HORIZONTAL = 0x01,
        MIRROR_VERTICAL = 0x02,
        MIRROR_ORIGIN = 0x03,
    } MIRROR_IMAGE;

    /**
     * @brief Line style enumeration for drawing
     * Available styles: LINE_SOLID, LINE_DOTTED, LINE_DASHED
     */
    typedef enum
    {
        LINE_SOLID = 0,
        LINE_DOTTED = 1,
        // LINE_DASHED = 2
    } LINE_STYLE;

    /**
     * @brief Fill mode enumeration for shapes
     * Available modes: DRAW_EMPTY (outline only), DRAW_FULL (filled)
     */
    typedef enum
    {
        DRAW_EMPTY = 0,
        DRAW_FULL = 1
    } DRAW_FILL;

    /**
     * @brief Available font sizes
     * Font8, Font12, Font16, Font20, Font24
     */
    static sFONT Font8;
    static sFONT Font12;
    static sFONT Font16;
    static sFONT Font20;
    static sFONT Font24;

    /**
     * @brief Constructor with pin parameters
     * @param busy_pin BUSY signal pin
     * @param rst_pin RST signal pin
     * @param dc_pin DC signal pin
     * @param cs_pin CS signal pin
     * @param clk_pin CLK signal pin (SCK)
     * @param din_pin DIN signal pin (MOSI)
     */
    EPDDisplay(int busy_pin, int rst_pin, int dc_pin, int cs_pin, int clk_pin, int din_pin);

    /**
     * @brief Destructor - frees allocated memory
     */
    ~EPDDisplay();

    /** ***************************************
    HARDWARE FUNCTIONS
    *****************************************/
    /**
     * @brief Initialize the display and allocate memory for buffers
     * @return true if initialization is successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Reset the display hardware
     */
    void reset();

    /**
     * @brief Clear the display (white)
     */
    void clear();

    /**
     * @brief Refresh the display to show the current buffer content
     */
    void display();

    /**
     * @brief Put the display into sleep mode to save power
     */
    void sleep();

    /**
     * @brief Check if the display is currently in sleep mode
     * @return true if display is in sleep mode, false otherwise
     */
    bool isInSleep();

    /**
     * @brief Wake up the display from sleep mode
     * If display is in sleep, performs a reset to wake it up
     * If not in sleep, does nothing
     */
    void wakeUp();

    /** ***************************************
    BASIC FUNCTIONS
    *****************************************/
    /**
     * @brief Draw a pixel at (x, y) with specified color
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     */
    void drawPixel(uint16_t x, uint16_t y, COLOR color);
    /**
     * @brief Fill the entire screen with specified color
     * @param color Color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     */
    void fillScreen(COLOR color);

    /**
     * @brief Set display rotation
     * @param rotate Rotation angle (EPDDisplay::ROTATE_0, EPDDisplay::ROTATE_90, EPDDisplay::ROTATE_180, EPDDisplay::ROTATE_270)
     */
    void setRotation(uint8_t rotate);

    /**
     * @brief Set display mirroring
     * @param mirror Mirroring mode (EPDDisplay::MIRROR_NONE, EPDDisplay::MIRROR_HORIZONTAL, EPDDisplay::MIRROR_VERTICAL, EPDDisplay::MIRROR_ORIGIN)
     */
    void setMirror(uint8_t mirror);

    /**
     * @brief Draw a bitmap image at (x, y) with specified width and height
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param width Width of the bitmap in pixels
     * @param height Height of the bitmap in pixels
     * @param bitmap Pointer to bitmap data array (1 bit per pixel, 0=black, 1=white)
     */
    void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap);

    /**
     * @brief Draw a bitmap image at (x, y) with specified width and height
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param width Width of the bitmap in pixels
     * @param height Height of the bitmap in pixels
     * @param bitmap Pointer to bitmap data array (1 bit per pixel, 0=black, 1=white)
     * @param active_color Color to use for '1' bits (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param inactive_color Color to use for '0' bits (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * Example: To draw a red bitmap on a white background:
     *   display.drawBitmap(x, y, width, height, bitmap, EPDDisplay::RED, EPDDisplay::WHITE);
     */
    void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap, COLOR active_color, COLOR inactive_color);

    /** ***************************************
    Shapes FUNCTIONS
    *****************************************/

    /**
     * @brief Draw a circle on the display
     * @param Xcenter X coordinate of circle center
     * @param Ycenter Y coordinate of circle center
     * @param radius Circle radius in pixels
     * @param color Color of the circle (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the circle outline in pixels
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawCircle(uint16_t Xcenter, uint16_t Ycenter, uint16_t radius, COLOR color, uint8_t line_width, DRAW_FILL draw_fill);

    /**
     * @brief Draw a rectangle on the display
     * @param Xstart X coordinate of top-left corner
     * @param Ystart Y coordinate of top-left corner
     * @param Xend X coordinate of bottom-right corner
     * @param Yend Y coordinate of bottom-right corner
     * @param color Color of the rectangle (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the rectangle outline in pixels
     * @param line_style Style of the lines (EPDDisplay::LINE_SOLID, EPDDisplay::LINE_DOTTED, EPDDisplay::LINE_DASHED)
     * @param draw_Fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, COLOR color, uint8_t line_width, LINE_STYLE line_style, DRAW_FILL draw_Fill);

    /**
     * @brief Draw a line on the display
     * @param Xstart X coordinate of line start point
     * @param Ystart Y coordinate of line start point
     * @param Xend X coordinate of line end point
     * @param Yend Y coordinate of line end point
     * @param color Color of the line (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the line in pixels
     * @param line_style Style of the line (EPDDisplay::LINE_SOLID, EPDDisplay::LINE_DOTTED, EPDDisplay::LINE_DASHED)
     */
    void drawLine(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, COLOR color, uint8_t line_width, LINE_STYLE line_style);

    /**
     * @brief Draw a point on the display
     * @param Xpoint X coordinate of the point
     * @param Ypoint Y coordinate of the point
     * @param color Color of the point (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param point_width Width/size of the point in pixels
     */
    void drawPoint(uint16_t Xpoint, uint16_t Ypoint, COLOR color, uint8_t point_width);

    /** ***************************************
    Complex Shapes FUNCTIONS
    *****************************************/

    /**
     * @brief Draw a rectangle with rounded corners on the display
     * @param Xstart X coordinate of top-left corner
     * @param Ystart Y coordinate of top-left corner
     * @param Xend X coordinate of bottom-right corner
     * @param Yend Y coordinate of bottom-right corner
     * @param radius Radius of the rounded corners in pixels
     * @param color Color of the rectangle (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the rectangle outline in pixels
     * @param line_style Style of the lines (EPDDisplay::LINE_SOLID, EPDDisplay::LINE_DOTTED, EPDDisplay::LINE_DASHED)
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawRoundedRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t radius, COLOR color, uint8_t line_width, LINE_STYLE line_style, DRAW_FILL draw_fill);

    /**
     * @brief Draw a star on the display
     * @param x_center X coordinate of star center
     * @param y_center Y coordinate of star center
     * @param radius_outer Outer radius of the star in pixels
     * @param radius_inner Inner radius of the star in pixels
     * @param num_points Number of star points (minimum 3, default 5)
     * @param color Color of the star (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the star outline in pixels
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawStar(uint16_t x_center, uint16_t y_center, uint16_t radius_outer, uint16_t radius_inner, uint8_t num_points, COLOR color, uint8_t line_width, DRAW_FILL draw_fill);

    /**
     * @brief Draw a triangle on the display
     * @param x1 X coordinate of first vertex
     * @param y1 Y coordinate of first vertex
     * @param x2 X coordinate of second vertex
     * @param y2 Y coordinate of second vertex
     * @param x3 X coordinate of third vertex
     * @param y3 Y coordinate of third vertex
     * @param color Color of the triangle (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the triangle outline in pixels
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, COLOR color, uint8_t line_width, DRAW_FILL draw_fill);

    /**
     * @brief Draw an ellipse on the display
     * @param x_center X coordinate of ellipse center
     * @param y_center Y coordinate of ellipse center
     * @param radius_x Horizontal radius in pixels
     * @param radius_y Vertical radius in pixels
     * @param color Color of the ellipse (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the ellipse outline in pixels
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawEllipse(uint16_t x_center, uint16_t y_center, uint16_t radius_x, uint16_t radius_y, COLOR color, uint8_t line_width, DRAW_FILL draw_fill);

    /**
     * @brief Draw a polygon on the display
     * @param points_x Array of X coordinates for polygon vertices
     * @param points_y Array of Y coordinates for polygon vertices
     * @param num_points Number of vertices in the polygon (minimum 3)
     * @param color Color of the polygon (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param line_width Width of the polygon outline in pixels
     * @param draw_fill Fill mode (EPDDisplay::DRAW_EMPTY, EPDDisplay::DRAW_FULL)
     */
    void drawPolygon(const uint16_t *points_x, const uint16_t *points_y, uint8_t num_points, COLOR color, uint8_t line_width, DRAW_FILL draw_fill);

    /** ***************************************
    Text FUNCTIONS
    *****************************************/

    /**
     * @brief Draw a single character on the display
     * @param Xpoint X coordinate for character position
     * @param Ypoint Y coordinate for character position
     * @param Acsii_Char ASCII character to display
     * @param Font Pointer to font structure (EPDDisplay::Font8, EPDDisplay::Font12, EPDDisplay::Font16, EPDDisplay::Font20, EPDDisplay::Font24)
     * @param color_foreground Foreground color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param color_background Background color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     */
    void drawChar(uint16_t Xpoint, uint16_t Ypoint, const char Acsii_Char, sFONT *Font, COLOR color_foreground, COLOR color_background);

    /**
     * @brief Draw a text string on the display
     * @param Xstart X coordinate for string start position
     * @param Ystart Y coordinate for string start position
     * @param pString Pointer to null-terminated string to display
     * @param Font Pointer to font structure (EPDDisplay::Font8, EPDDisplay::Font12, EPDDisplay::Font16, EPDDisplay::Font20, EPDDisplay::Font24)
     * @param color_foreground Foreground color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param color_background Background color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     */
    void drawString(uint16_t Xstart, uint16_t Ystart, const char *pString, sFONT *Font, COLOR color_foreground, COLOR color_background);

    /**
     * @brief Draw an integer number on the display
     * @param Xpoint X coordinate for number position
     * @param Ypoint Y coordinate for number position
     * @param Number Integer number to display
     * @param Font Pointer to font structure (EPDDisplay::Font8, EPDDisplay::Font12, EPDDisplay::Font16, EPDDisplay::Font20, EPDDisplay::Font24)
     * @param color_foreground Foreground color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param color_background Background color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     */
    void drawNumber(uint16_t Xpoint, uint16_t Ypoint, int32_t Number, sFONT *Font, COLOR color_foreground, COLOR color_background);

    /**
     * @brief Draw a floating-point number on the display
     * @param Xpoint X coordinate for number position
     * @param Ypoint Y coordinate for number position
     * @param Number Floating-point number to display
     * @param Font Pointer to font structure (EPDDisplay::Font8, EPDDisplay::Font12, EPDDisplay::Font16, EPDDisplay::Font20, EPDDisplay::Font24)
     * @param color_foreground Foreground color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param color_background Background color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     */
    void drawFloat(uint16_t Xpoint, uint16_t Ypoint, float Number, sFONT *Font, COLOR color_foreground, COLOR color_background);

    /**
     * @brief Draw time in HH:MM:SS format on the display
     * @param Xstart X coordinate for time display start position
     * @param Ystart Y coordinate for time display start position
     * @param hour Hour value (0-23)
     * @param minute Minute value (0-59)
     * @param second Second value (0-59)
     * @param Font Pointer to font structure (EPDDisplay::Font8, EPDDisplay::Font12, EPDDisplay::Font16, EPDDisplay::Font20, EPDDisplay::Font24)
     * @param color_foreground Foreground color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param color_background Background color (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     */
    void drawTime(uint16_t Xstart, uint16_t Ystart, uint8_t hour, uint8_t minute, uint8_t second, sFONT *Font, COLOR color_foreground, COLOR color_background);

    /** ***************************************
    CLOCK FUNCTIONS
    *****************************************/

    /**
     * @brief Draw an analog clock on the display
     * @param x_center X coordinate of clock center
     * @param y_center Y coordinate of clock center
     * @param radius Radius of the clock face
     * @param hour Hour value (0-11 for 12-hour format, 0-23 for 24-hour format)
     * @param minute Minute value (0-59)
     * @param second Second value (0-59)
     * @param color_face Color of the clock face (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param color_hands Color of the clock hands (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param color_numbers Color of the hour numbers (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param show_seconds Whether to draw the seconds hand
     * @param show_numbers Whether to draw hour numbers (1-12)
     */
    void drawAnalogClock(uint16_t x_center, uint16_t y_center, uint16_t radius, uint8_t hour, uint8_t minute, uint8_t second, COLOR color_face, COLOR color_hands, COLOR color_numbers, bool show_seconds = true, bool show_numbers = true);

    /**
     * @brief Draw a 7-segment style digital clock on the display
     * @param x_start X coordinate for clock start position
     * @param y_start Y coordinate for clock start position
     * @param segment_width Width of each segment
     * @param segment_height Height of each segment
     * @param hour Hour value (0-23)
     * @param minute Minute value (0-59)
     * @param second Second value (0-59)
     * @param color_on Color for active segments (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED)
     * @param color_off Color for inactive segments (EPDDisplay::WHITE, EPDDisplay::BLACK, EPDDisplay::RED, EPDDisplay::NULL_COLOR)
     * @param show_seconds Whether to display seconds
     * @param format_24h Whether to use 24-hour format (true) or 12-hour format (false)
     */
    void drawDigitalClock7Segment(uint16_t x_start, uint16_t y_start, uint16_t segment_width, uint16_t segment_height, uint8_t hour, uint8_t minute, uint8_t second, COLOR color_on, COLOR color_off, bool show_seconds = true, bool format_24h = true);

private:
    /** ***************************************
    VARIABLES
    *****************************************/

    uint8_t *blackBuffer;
    uint8_t *redBuffer;
    bool isInitialized;
    bool isSleep;

    uint16_t width;
    uint16_t height;
    uint8_t widthByte;
    uint16_t heightByte;
    uint16_t widthMemory;
    uint16_t heightMemory;
    uint8_t rotate;
    uint8_t mirror;

    // Pins
    int m_BUSY_pin;
    int m_RST_pin;
    int m_DC_pin;
    int m_CS_pin;
    int m_CLK_pin;
    int m_DIN_pin;

    /*****************************************
    HARDWARE FUNCTIONS
    *****************************************/

    /**
     * @brief Check if display is ready for operations (initialized and not in sleep)
     * @return true if ready, false otherwise (with debug message)
     */
    bool checkDisplayReady();

    /**
     * @brief Send command to EPD controller
     * @param Reg Command register value to send
     */
    void SendCommand(uint8_t Reg);

    /**
     * @brief Send data to EPD controller
     * @param Data Data byte to send
     */
    void SendData(uint8_t Data);

    /**
     * @brief Wait for busy signal to clear
     */
    void ReadBusy();

    /**
     * @brief Write a byte via SPI
     * @param value Byte value to write
     */
    void SPI_WriteByte(uint8_t value);

    /*****************************************
    Utils FUNCTIONS
    *****************************************/

    /**
     * @brief Clear the red color buffer
     */
    void ClearRed();

    /**
     * @brief Clear the black color buffer
     */
    void ClearBlack();

    /**
     * @brief Draw an arc corner for rounded rectangles
     * @param x_center X coordinate of arc center
     * @param y_center Y coordinate of arc center
     * @param radius Radius of the arc
     * @param corner Corner to draw (bitfield: 1=top-right, 2=top-left, 4=bottom-right, 8=bottom-left)
     * @param color Color of the arc
     * @param line_width Width of the arc line
     */
    void drawArcCorner(uint16_t x_center, uint16_t y_center, uint16_t radius, uint8_t corner, COLOR color, uint8_t line_width);

    /**
     * @brief Draw a single 7-segment digit
     * @param x X coordinate of digit position
     * @param y Y coordinate of digit position
     * @param digit Digit to draw (0-9)
     * @param segment_width Width of each segment
     * @param segment_height Height of each segment
     * @param color_on Color for active segments
     * @param color_off Color for inactive segments
     */
    void draw7SegmentDigit(uint16_t x, uint16_t y, uint8_t digit, uint16_t segment_width, uint16_t segment_height, COLOR color_on, COLOR color_off);

    /**
     * @brief Draw a single segment of a 7-segment display
     * @param x X coordinate of segment
     * @param y Y coordinate of segment
     * @param segment_type Type of segment (0-6: a,b,c,d,e,f,g)
     * @param segment_width Width of the segment
     * @param segment_height Height of the segment
     * @param color Color of the segment
     */
    void draw7Segment(uint16_t x, uint16_t y, uint8_t segment_type, uint16_t segment_width, uint16_t segment_height, COLOR color);
};

#endif // __EPDDISPLAY_H