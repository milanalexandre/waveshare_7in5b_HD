#include <Arduino.h>
#include "EPDDisplay.h"
#include "image.cpp"


#define BUSY_pin 4
#define RST_pin 16
#define DC_pin 17
#define CS_pin 5
#define CLK_pin 18
#define DIN_pin 23

EPDDisplay display(BUSY_pin, RST_pin, DC_pin, CS_pin, CLK_pin, DIN_pin);

// Variables for page navigation
int currentPage = 0;
const int totalPages = 11;
unsigned long lastPageChange = 0;
const unsigned long pageInterval = 5000; // 5 seconds per page

void testBasicShapes();
void testRoundedRectangles();
void testStars();
void testTriangles();
void testEllipses();
void testPolygons();
void testLineStyles();
void testTextAndNumbers();
void testLargeBitmap();
void testAnalogClock();
void testDigitalClock();
void showCurrentPage();

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing EPD shapes test...");

  if (!display.initialize())
  {
    Serial.println("Screen initialization error");
    return;
  }

  Serial.println("Screen successfully initialized");
  display.fillScreen(EPDDisplay::WHITE);
  showCurrentPage();
}

void loop()
{
  // Change page automatically every 5 seconds
  if (millis() - lastPageChange > pageInterval)
  {
    currentPage = (currentPage + 1) % totalPages;
    showCurrentPage();
    lastPageChange = millis();
  }
}

void showCurrentPage()
{
  Serial.printf("Displaying page %d/%d\n", currentPage + 1, totalPages);

  // Clear screen
  display.fillScreen(EPDDisplay::WHITE);

  // Page title
  char title[50];
  sprintf(title, "Page %d/%d", currentPage + 1, totalPages);
  display.drawString(10, 10, title, &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  switch (currentPage)
  {
  case 0:
    testBasicShapes();
    break;
  case 1:
    testRoundedRectangles();
    break;
  case 2:
    testStars();
    break;
  case 3:
    testTriangles();
    break;
  case 4:
    testEllipses();
    break;
  case 5:
    testPolygons();
    break;
  case 6:
    testLineStyles();
    break;
  case 7:
    testTextAndNumbers();
    break;
  case 8:
    testLargeBitmap();
    break;
  case 9:
    testAnalogClock();
    break;
  case 10:
    testDigitalClock();
    break;
  }

  display.display();
}

void testBasicShapes()
{
  display.drawString(10, 40, "Basic shapes", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Circles
  display.drawCircle(100, 100, 30, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawCircle(200, 100, 25, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawCircle(300, 100, 20, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawCircle(400, 100, 15, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawCircle(500, 100, 30, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawCircle(600, 100, 30, EPDDisplay::BLACK, 4, EPDDisplay::DRAW_EMPTY);
  display.drawCircle(700, 100, 30, EPDDisplay::BLACK, 5, EPDDisplay::DRAW_EMPTY);
  display.drawString(80, 120, "Circles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Rectangles
  display.drawRectangle(50, 150, 120, 200, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(150, 150, 220, 200, EPDDisplay::RED, 2, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(250, 150, 320, 200, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
  display.drawRectangle(350, 150, 420, 200, EPDDisplay::RED, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
  display.drawRectangle(450, 150, 520, 200, EPDDisplay::BLACK, 5, EPDDisplay::LINE_DOTTED, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(550, 150, 620, 200, EPDDisplay::RED, 10, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(200, 210, "Rectangles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Lines
  display.drawLine(50, 220, 150, 220, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
  display.drawLine(50, 240, 150, 240, EPDDisplay::RED, 3, EPDDisplay::LINE_SOLID);
  display.drawLine(200, 220, 300, 240, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);
  display.drawLine(350, 220, 450, 240, EPDDisplay::RED, 2, EPDDisplay::LINE_DOTTED);
  display.drawString(250, 250, "Lines", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Points
  display.drawPoint(100, 280, EPDDisplay::BLACK, 3);
  display.drawPoint(150, 280, EPDDisplay::RED, 5);
  display.drawPoint(200, 280, EPDDisplay::BLACK, 7);
  display.drawPoint(250, 280, EPDDisplay::RED, 9);
  display.drawString(150, 300, "Points", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void testRoundedRectangles()
{
  display.drawString(10, 40, "Rounded rectangles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Different radii
  display.drawRoundedRectangle(50, 80, 150, 130, 5, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(70, 140, "r=5", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawRoundedRectangle(180, 80, 280, 130, 10, EPDDisplay::RED, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(200, 140, "r=10", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawRoundedRectangle(310, 80, 410, 130, 15, EPDDisplay::BLACK, 2, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(330, 140, "r=15", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawRoundedRectangle(440, 80, 540, 130, 20, EPDDisplay::RED, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(460, 140, "r=20", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Filled versions
  display.drawRoundedRectangle(50, 180, 150, 230, 8, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
  display.drawString(60, 240, "Black filled", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

  display.drawRoundedRectangle(180, 180, 280, 230, 12, EPDDisplay::RED, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_FULL);
  display.drawString(190, 240, "Red filled", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

  // Different line thickness
  display.drawRoundedRectangle(310, 180, 410, 230, 10, EPDDisplay::BLACK, 3, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(320, 240, "Thick line", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawRoundedRectangle(440, 180, 540, 230, 10, EPDDisplay::RED, 1, EPDDisplay::LINE_DOTTED, EPDDisplay::DRAW_EMPTY);
  display.drawString(450, 240, "Dotted", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Large shapes
  display.drawRoundedRectangle(100, 280, 400, 350, 25, EPDDisplay::BLACK, 2, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawString(200, 310, "Large shape r=25", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void testStars()
{
  display.drawString(10, 40, "Stars", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // 5-pointed stars with different sizes
  display.drawStar(100, 120, 40, 20, 5, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(80, 170, "5 points", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawStar(250, 120, 35, 15, 4, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(230, 170, "4 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawStar(400, 120, 30, 12, 6, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(380, 170, "6 points", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawStar(550, 120, 25, 10, 8, EPDDisplay::RED, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(530, 170, "8 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawStar(650, 120, 25, 10, 10, EPDDisplay::RED, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(630, 170, "10 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Filled stars
  display.drawStar(100, 250, 40, 20, 5, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(80, 300, "5 points", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawStar(250, 250, 35, 15, 4, EPDDisplay::RED, 2, EPDDisplay::DRAW_FULL);
  display.drawString(230, 300, "4 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawStar(400, 250, 30, 12, 6, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(380, 300, "6 points", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawStar(550, 250, 25, 10, 8, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawString(530, 300, "8 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawStar(650, 250, 25, 10, 10, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawString(630, 300, "10 points", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Different inner/outer radius ratios
  display.drawStar(100, 380, 40, 5, 5, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(70, 430, "Ratio 1:8", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawStar(250, 380, 40, 30, 5, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(220, 430, "Ratio 3:4", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawStar(400, 380, 40, 35, 5, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(370, 430, "Ratio 7:8", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void testTriangles()
{
  display.drawString(10, 40, "Triangles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawTriangle(100, 80, 150, 150, 50, 150, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(70, 160, "Equilateral", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawTriangle(200, 80, 200, 150, 270, 150, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(210, 160, "Right", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawTriangle(350, 80, 320, 150, 380, 150, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(330, 160, "Isosceles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawTriangle(450, 90, 480, 140, 420, 160, EPDDisplay::RED, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(430, 170, "Scalene", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Filled triangles
  display.drawTriangle(100, 220, 150, 290, 50, 290, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(70, 300, "Equilateral", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawTriangle(200, 220, 200, 290, 270, 290, EPDDisplay::RED, 2, EPDDisplay::DRAW_FULL);
  display.drawString(210, 300, "Right", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawTriangle(350, 220, 320, 290, 380, 290, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(330, 300, "Isosceles", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawTriangle(450, 220, 480, 280, 420, 300, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawString(430, 300, "Scalene", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Large triangle
  display.drawTriangle(250, 350, 150, 450, 350, 450, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(220, 460, "Large triangle", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
}

void testEllipses()
{
  display.drawString(10, 40, "Ellipses", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Horizontal ellipse
  display.drawEllipse(120, 100, 50, 25, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(90, 130, "Horizontal", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Vertical ellipse
  display.drawEllipse(280, 100, 25, 50, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(260, 160, "Vertical", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Circle (perfect ellipse)
  display.drawEllipse(440, 100, 35, 35, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(420, 140, "Circle", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Very flat ellipse
  display.drawEllipse(600, 100, 60, 10, EPDDisplay::RED, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(570, 120, "Flat", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Filled ellipses
  display.drawEllipse(150, 230, 45, 25, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_FULL);
  display.drawString(120, 260, "Black filled", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

  display.drawEllipse(350, 230, 25, 45, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawString(320, 280, "Red filled", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

  // Different sizes
  display.drawEllipse(120, 350, 30, 20, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawEllipse(250, 350, 40, 30, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawEllipse(400, 350, 50, 40, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawEllipse(580, 350, 60, 50, EPDDisplay::RED, 3, EPDDisplay::DRAW_EMPTY);

  display.drawString(200, 400, "Increasing sizes", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void testPolygons()
{
  display.drawString(10, 40, "Polygons", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Hexagon
  uint16_t hex_x[] = {150, 180, 180, 150, 120, 120};
  uint16_t hex_y[] = {80, 95, 125, 140, 125, 95};
  display.drawPolygon(hex_x, hex_y, 6, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(130, 150, "Hexagon", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Pentagon
  uint16_t pent_x[] = {300, 320, 310, 290, 280};
  uint16_t pent_y[] = {80, 100, 130, 130, 100};
  display.drawPolygon(pent_x, pent_y, 5, EPDDisplay::RED, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(280, 140, "Pentagon", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Octagon
  uint16_t oct_x[] = {470, 490, 500, 500, 490, 470, 450, 440, 440};
  uint16_t oct_y[] = {80, 80, 90, 110, 120, 120, 110, 90, 80};
  display.drawPolygon(oct_x, oct_y, 8, EPDDisplay::BLACK, 1, EPDDisplay::DRAW_EMPTY);
  display.drawString(450, 130, "Octagon", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Diamond
  uint16_t diamond_x[] = {150, 180, 150, 120};
  uint16_t diamond_y[] = {200, 230, 260, 230};
  display.drawPolygon(diamond_x, diamond_y, 4, EPDDisplay::RED, 1, EPDDisplay::DRAW_FULL);
  display.drawString(130, 270, "Diamond", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::NULL_COLOR);

  // Irregular shape
  uint16_t irreg_x[] = {300, 350, 340, 320, 290, 280};
  uint16_t irreg_y[] = {200, 210, 240, 260, 250, 220};
  display.drawPolygon(irreg_x, irreg_y, 6, EPDDisplay::BLACK, 2, EPDDisplay::DRAW_EMPTY);
  display.drawString(290, 270, "Irregular", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
}

void testLineStyles()
{
  display.drawString(10, 40, "Line styles", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Solid lines with different thickness
  display.drawString(50, 80, "Solid lines:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawLine(50, 100, 250, 100, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
  display.drawString(260, 95, "Thickness 1", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawLine(50, 120, 250, 120, EPDDisplay::RED, 2, EPDDisplay::LINE_SOLID);
  display.drawString(260, 115, "Thickness 2", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  display.drawLine(50, 140, 250, 140, EPDDisplay::BLACK, 3, EPDDisplay::LINE_SOLID);
  display.drawString(260, 135, "Thickness 3", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawLine(50, 160, 250, 160, EPDDisplay::RED, 5, EPDDisplay::LINE_SOLID);
  display.drawString(260, 155, "Thickness 5", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Dotted lines
  display.drawString(50, 200, "Dotted lines:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawLine(50, 220, 250, 220, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);
  display.drawString(260, 215, "Black dotted", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawLine(50, 240, 250, 240, EPDDisplay::RED, 2, EPDDisplay::LINE_DOTTED);
  display.drawString(260, 235, "Red dotted", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);

  // Diagonal lines
  display.drawString(50, 280, "Diagonal lines:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawLine(50, 300, 150, 350, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID);
  display.drawLine(70, 300, 170, 350, EPDDisplay::RED, 2, EPDDisplay::LINE_SOLID);
  display.drawLine(90, 300, 190, 350, EPDDisplay::BLACK, 1, EPDDisplay::LINE_DOTTED);
  display.drawLine(110, 300, 210, 350, EPDDisplay::RED, 2, EPDDisplay::LINE_DOTTED);

  // Shapes with different styles
  display.drawString(350, 80, "Shapes with styles:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawRectangle(400, 100, 500, 150, EPDDisplay::BLACK, 1, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(520, 100, 620, 150, EPDDisplay::RED, 2, EPDDisplay::LINE_DOTTED, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(400, 170, 500, 220, EPDDisplay::BLACK, 3, EPDDisplay::LINE_SOLID, EPDDisplay::DRAW_EMPTY);
  display.drawRectangle(520, 170, 620, 220, EPDDisplay::RED, 1, EPDDisplay::LINE_DOTTED, EPDDisplay::DRAW_EMPTY);
}

void testTextAndNumbers()
{
  display.drawString(10, 40, "Text and numbers", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Different font sizes
  display.drawString(50, 80, "Font12", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(150, 80, "Font12", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawString(250, 80, "Font16", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(350, 80, "Font20", &EPDDisplay::Font20, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawString(450, 80, "Font24", &EPDDisplay::Font24, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Text with background
  display.drawString(50, 130, "Black on white", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::WHITE);
  display.drawString(200, 130, "White on black", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::BLACK);
  display.drawString(350, 130, "White on red", &EPDDisplay::Font12, EPDDisplay::WHITE, EPDDisplay::RED);
  display.drawString(500, 130, "Red on white", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::WHITE);

  // Integer numbers
  display.drawString(50, 170, "Integer numbers:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawNumber(50, 190, 12345, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawNumber(150, 190, -6789, &EPDDisplay::Font16, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawNumber(250, 190, 0, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Floating point numbers
  display.drawString(50, 230, "Decimal numbers:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawFloat(50, 250, 123.45, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawFloat(180, 250, -67.89, &EPDDisplay::Font16, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawFloat(320, 250, 3.14159, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Time display
  display.drawString(50, 290, "Time display:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawTime(50, 310, 14, 25, 36, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawTime(250, 310, 9, 5, 0, &EPDDisplay::Font16, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawTime(420, 310, 23, 59, 59, &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Simulated multiline text
  display.drawString(50, 360, "Line 1 - Long text that could", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(50, 380, "Line 2 - extend over multiple", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(50, 400, "Line 3 - lines for demonstration", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Special characters
  display.drawString(50, 440, "Chars: !@#$%^&*()_+-=[]{}|;':\",./<>?", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
}

void testLargeBitmap()
{
  display.drawString(10, 40, "Large bitmap test - 440x440px", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawString(50, 80, "Testing large bitmap rendering:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(50, 100, "Size: 440x440 pixels", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  display.drawBitmap(220, 60, 440, 440, epd_bitmap_440x440_red, EPDDisplay::WHITE, EPDDisplay::RED);
  display.drawBitmap(220, 60, 440, 440, epd_bitmap_440x440_black, EPDDisplay::NULL_COLOR, EPDDisplay::BLACK);
}

void testAnalogClock()
{
  display.drawString(10, 40, "Analog Clock", &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Example of analog clock - 10:30:45
  uint8_t hour = 10;
  uint8_t minute = 30;
  uint8_t second = 45;

  // Position and size of the main clock
  uint16_t x_center = 440; // Centered horizontally
  uint16_t y_center = 280; // Centered vertically
  uint16_t radius = 160;

  // Draw the main analog clock
  display.drawAnalogClock(
      x_center, y_center, radius,
      hour, minute, second,
      EPDDisplay::BLACK, // Dial color
      EPDDisplay::BLACK, // Hands color
      EPDDisplay::RED,   // Numbers color
      true,              // Show second hand (red)
      true               // Show hour numbers
  );

  // Clock information
  display.drawString(50, 80, "Clock with:", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(50, 100, "- Hour/minute hands (black)", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawString(50, 120, "- Second hand (red)", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawString(50, 140, "- Numbers 1-12 (red)", &EPDDisplay::Font12, EPDDisplay::RED, EPDDisplay::NULL_COLOR);
  display.drawString(50, 160, "- Minute marks", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Also display the time in digital format
  display.drawString(50, 200, "Displayed time:", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawTime(50, 225, hour, minute, second, &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Small example clock without seconds
  uint16_t x_small = 150;
  uint16_t y_small = 350;
  uint16_t radius_small = 80;

  display.drawString(50, 300, "Simplified clock:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);
  display.drawAnalogClock(
      x_small, y_small, radius_small,
      15, 45, 0, // 3:45 PM
      EPDDisplay::BLACK,
      EPDDisplay::BLACK,
      EPDDisplay::NULL_COLOR, // No numbers
      false,                  // No second hand
      false                   // No numbers
  );
}

void testDigitalClock()
{
  display.drawString(10, 40, "7 Segment Digital Clock", &EPDDisplay::Font20, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  // Example 1: 24h clock with seconds
  uint8_t hour1 = 14;
  uint8_t minute1 = 25;
  uint8_t second1 = 30;

  display.drawString(50, 80, "24h format with seconds:", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  uint16_t x_start1 = 100;
  uint16_t y_start1 = 110;
  uint16_t seg_width1 = 35;
  uint16_t seg_height1 = 70;

  display.drawDigitalClock7Segment(
      x_start1, y_start1,
      seg_width1, seg_height1,
      hour1, minute1, second1,
      EPDDisplay::BLACK,      // Active segments in black
      EPDDisplay::NULL_COLOR, // Inactive segments invisible
      true,                   // Show seconds
      true                    // 24h format
  );

  // Example 2: 12h clock without seconds
  uint8_t hour2 = 15; // 3 PM
  uint8_t minute2 = 45;
  uint8_t second2 = 0;

  display.drawString(50, 220, "12h format without seconds:", &EPDDisplay::Font16, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  uint16_t x_start2 = 120;
  uint16_t y_start2 = 250;
  uint16_t seg_width2 = 30;
  uint16_t seg_height2 = 60;

  display.drawDigitalClock7Segment(
      x_start2, y_start2,
      seg_width2, seg_height2,
      hour2, minute2, second2,
      EPDDisplay::RED,        // Active segments in red
      EPDDisplay::NULL_COLOR, // Inactive segments invisible
      false,                  // No seconds
      false                   // 12h format with AM/PM
  );

  // Example 3: Small minimalist clock
  display.drawString(50, 360, "Mini clock:", &EPDDisplay::Font12, EPDDisplay::BLACK, EPDDisplay::NULL_COLOR);

  uint16_t x_start3 = 50;
  uint16_t y_start3 = 380;
  uint16_t seg_width3 = 20;
  uint16_t seg_height3 = 40;

  display.drawDigitalClock7Segment(
      x_start3, y_start3,
      seg_width3, seg_height3,
      9, 5, 0, // 9:05
      EPDDisplay::BLACK,
      EPDDisplay::NULL_COLOR,
      false, // No seconds
      false  // 12h format
  );
}