#include <Adafruit_NeoPixel.h>

// config for LED strip
#define ledPin        D7
#define numLEDs       40  // number of LEDs used in strip, needs to be an even number
#define maxBright     255 // 0-255 max brightness
#define IDLE_COLOR    pixels.Color(0, 0, 0)   // BLACK or OFF
#define STOP_COLOR    pixels.Color(255, 0, 0) // RED
#define GO_COLOR      pixels.Color(0, 255, 0) // GREEN
#define BACK_COLOR    pixels.Color(0, 0, 128)  // faint BLUE

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);
float pixelSize = 1.0;    // the physical distance that each LED represents in cm

// Arduino setup function. Runs in CPU 1
void setup()
{
  Serial.begin(74880);  // default baud rate for bootloader
  Serial.printf("\n\nSMT32F103 startup...\n");

 // set up LED strip
  pixels.begin();
  pixels.setBrightness(maxBright);  // to prevent overcurrent situation, start low
  runningLEDs();
  pixels.rainbow();
  pixels.show();
}

// Arduino loop function. Runs in CPU 1.
void loop()
{
  processPixels();
}

void processPixels()
{
  pixels.fill(IDLE_COLOR, 0, numLEDs);
  pixels.show();
}

void runningLEDs()
{
  long off = pixels.Color(0, 0, 0);
  for (int colorLoop = 0; colorLoop < 3; colorLoop++)
  {
    long color = pixels.Color((colorLoop == 0 ? 255 : 0), (colorLoop == 1 ? 255 : 0), (colorLoop == 2 ? 255 : 0));
    for (int count = 0; count < pixels.numPixels(); count++)
    {
      pixels.fill(off, 0, pixels.numPixels());
      pixels.setPixelColor((colorLoop%2 == 0 ? count : pixels.numPixels() - count - 1), color);
      pixels.show();
      delay(20);
    }
  }
  pixels.fill(off, 0, pixels.numPixels());
  pixels.show();
}
