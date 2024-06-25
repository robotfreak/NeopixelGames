#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t orange = strip.Color(128, 255, 0);
uint32_t red = strip.Color(255, 0, 0);
int bri = 50;
// Fill the dots one after the other with a color
void topLeft() {
  for (uint16_t i = 0; i < 8; i++) {
    strip.setPixelColor(i, green);
    strip.setBrightness(bri);
    strip.show();
  }
}
void botLeft() {
  for (uint16_t i = 8; i < 16; i++) {
    strip.setPixelColor(i, blue);
    strip.setBrightness(bri);
    strip.show();
  }
}
void botRight() {
  for (uint16_t i = 16; i < 24; i++) {
    strip.setPixelColor(i, yellow);
    strip.setBrightness(bri);
    strip.show();
  }
}
void topRight() {
  for (uint16_t i = 24; i < 32; i++) {
    strip.setPixelColor(i, red);
    strip.setBrightness(bri);
    strip.show();
  }
}

void pixelsOff() {
  for (uint16_t i = 0; i < strip.numPixels(); i = i + 1) {
    strip.setPixelColor(i, 0);  //turn every pixel off
  }
  strip.show();  // Initialize all pixels to 'off'
}

void gameOver() {
  for (uint16_t i = 0; i < 32; i++) {
    strip.setPixelColor(i, red);
    strip.setBrightness(bri);
    strip.show();
  }
}

void victory() {
  for (uint16_t i = 0; i < 32; i++) {
    strip.setPixelColor(i, green);
    strip.setBrightness(bri);
    strip.show();
  }
}

void colorCycle(uint8_t wait) {
  for (uint16_t i = 0; i < 8; i++) {
    strip.setPixelColor(i, green);
    strip.setBrightness(bri);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 8; i < 16; i++) {
    strip.setPixelColor(i, blue);
    strip.setBrightness(bri);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 16; i < 24; i++) {
    strip.setPixelColor(i, yellow);
    strip.setBrightness(bri);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 24; i < 32; i++) {
    strip.setPixelColor(i, red);
    strip.setBrightness(bri);
    strip.show();
    delay(wait);
  }
}
void colorCircle(uint32_t color) {
  for (uint16_t i = 0; i < 32; i++) {
    strip.setPixelColor(i, color);
    strip.setBrightness(bri);
    strip.show();
  }
}
