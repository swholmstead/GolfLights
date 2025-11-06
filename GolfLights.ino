#include <Adafruit_NeoPixel.h>

// config for wiring harness
#define leftPin       D6
#define rightPin      D7
#define reversePin    D5
#define brakePin      D2

// config for LED strip
#define ledPin        D1
#define numLEDs       36  // number of LEDs used in strip, needs to be an even number
#define maxBright     255 // 0-255 max brightness; to prevent overcurrent, start low
#define OFF_COLOR     pixels.Color(0, 0, 0)        // OFF
#define IDLE_COLOR    pixels.Color(0, 64, 64)      // POWER ON
#define STOP_COLOR    pixels.Color(255, 0, 0)      // RED
#define TURN_COLOR    pixels.Color(255, 191, 0)    // YELLOW
#define REVERSE_COLOR pixels.Color(128, 128, 128)  // WHITE
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);
float pixelSize = 1.0;    // the physical distance that each LED represents in cm

// general config
int leftPosition = -1;
int rightPosition = -1;

// Arduino setup function. Runs in CPU 1
void setup()
{
  // set up wiring harness
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
  pinMode(reversePin, INPUT);
  pinMode(brakePin, INPUT);

  // set up LED strip
  pixels.begin();
  pixels.setBrightness(maxBright);
  // runningLEDs();
  pixels.fill(IDLE_COLOR, 0, pixels.numPixels());
  pixels.show();
}

// Arduino loop function. Runs in CPU 1.
void loop()
{
  processPixels();
}

void processPixels()
{
  // default
  pixels.fill(IDLE_COLOR, 0, pixels.numPixels());
  // check for brakes
  if (isPinHigh(brakePin))
  {
    pixels.fill(STOP_COLOR, 0, pixels.numPixels());
  }
  // check for reverse
  if (isPinHigh(reversePin))
  {
    pixels.fill(REVERSE_COLOR, 0, pixels.numPixels());
  }
  // check for left turn
  if (isPinHigh(leftPin))
  {
    rightPosition = -1;
    for (int position = 0; position < pixels.numPixels()/2; position++)
    {
      pixels.setPixelColor(pixels.numPixels()/2 - position - 1, (position <= leftPosition+1 ? TURN_COLOR : OFF_COLOR));
    }
    leftPosition++;
    if (leftPosition >= pixels.numPixels()/2 - 1)
    {
      leftPosition = -1;
    }
  }
  // check for right turn
  if (isPinHigh(rightPin))
  {
    leftPosition = -1;
    for (int position = 0; position < pixels.numPixels()/2; position++)
    {
      pixels.setPixelColor(pixels.numPixels()/2 + position, (position <= rightPosition+1 ? TURN_COLOR : OFF_COLOR));
    }
    rightPosition++;
    if (rightPosition >= pixels.numPixels()/2 - 1)
    {
      rightPosition = -1;
    }
  }
  pixels.show();
  delay(1000/pixels.numPixels());
}

bool isPinHigh(int pin)
{
  return digitalRead(pin) == HIGH;
}

void runningLEDs()
{
  long off = pixels.Color(0, 0, 0);
  for (int colorLoop = 0; colorLoop < 3; colorLoop++)
  {
    long color = pixels.Color((colorLoop == 0 ? 255 : 0), (colorLoop == 1 ? 255 : 0), (colorLoop == 2 ? 255 : 0));
    for (int count = 0; count < pixels.numPixels(); count++)
    {
      pixels.clear();
      pixels.setPixelColor((colorLoop%2 == 0 ? count : pixels.numPixels() - count - 1), color);
      pixels.show();
      delay(15);
    }
  }
  pixels.clear();
  pixels.show();
}
