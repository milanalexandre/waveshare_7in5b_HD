#include <Arduino.h>
#include "EPDDisplay.h"

// Pin configuration for EPD display
// These pins are defined via build_flags in platformio.ini:
// BUSY_pin, RST_pin, DC_pin, CS_pin, CLK_pin, DIN_pin

void setup()
{
  Serial.begin(115200);
  Serial.println("EPDDisplay Library - Ready to use!");
  Serial.println("Check the examples in the 'examples' folder for complete demonstrations.");
  Serial.println("Pin configuration:");
  Serial.printf("  BUSY: %d\n", BUSY_pin);
  Serial.printf("  RST:  %d\n", RST_pin);
  Serial.printf("  DC:   %d\n", DC_pin);
  Serial.printf("  CS:   %d\n", CS_pin);
  Serial.printf("  CLK:  %d\n", CLK_pin);
  Serial.printf("  DIN:  %d\n", DIN_pin);
}

void loop()
{
  // Minimal example code - the library is ready to use
  delay(1000);
}
