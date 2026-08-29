// CONFIG PARAMETERS
#define reverseActiveLow 1 // for Star EV, comment out for Yamaha
#define highDefLed 1 // 144 pixels/m
// #define backupBuzzer 1 // software reverse buzzer
#define animateTurn 1 // moving LEDs for turn signals

#include <Adafruit_NeoPixel.h>

// config for wiring harness
#define leftPin       D7
#define rightPin      D6
#define reversePin    D2
#define brakePin      D0
#define lightsPin     D5

// config for LED strip
#define buzzerPin     D1
#define ledPin        D3
#ifdef highDefLed
#define numLEDs      116 // number of LEDs used in strip, needs to be an even number
#define maxBright    127 // 0-255 max brightness; to prevent overcurrent, start low
#else
#define numLEDs       44 // standard 30 inches
#define maxBright    255 // 0-255 max brightness; to prevent overcurrent, start low
#endif
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);
int blinkRate = 300;
unsigned long idleColor;
unsigned long stopColor;
unsigned long turnColor;
unsigned long reverseColor;
unsigned long lightsColor;

// general config
unsigned int leftPosition = 0;
unsigned int rightPosition = 0;
unsigned int idlePosition = 0;
int idleDirection = 1;
int reverseCount = 0;
int reverseDelay = blinkRate / 25;
int reverseSize = numLEDs / 20;
#ifdef backupBuzzer
int buzzerCount = 0;
uint8_t buzzer = LOW;
uint8_t previousBuzzer = LOW;
#endif

// Arduino setup function. Runs in CPU 1
void setup()
{
  Serial.begin(74880);
  Serial.printf("\n\nStarting golf lights  LED: %d\n", numLEDs);

  idleColor = 0x000000;    // black
  stopColor = 0xff0000;    // red
  turnColor = 0xff7f00;    // amber
  reverseColor = 0xffffff; // white
  lightsColor = 0x200000;  // faint red
  Serial.printf("idle: %06lx  stop: %06lx  turn: %06lx  reverse: %06lx\n", idleColor, stopColor, turnColor, reverseColor);

  // set up wiring harness
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
#ifdef backupBuzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
#endif
#ifdef reverseActiveLow
  pinMode(reversePin, INPUT_PULLUP); // active low
#else
  pinMode(reversePin, INPUT);
#endif
  pinMode(brakePin, INPUT);
  pinMode(lightsPin, INPUT);

  // set up LED strip
  pixels.begin();
  pixels.setBrightness(maxBright);
  pixels.fill(idleColor, 0, numLEDs);
  pixels.show();
}

// Arduino loop function. Runs in CPU 1.
void loop()
{
  processPixels();
}

void processPixels()
{
  // check for reverse
#ifdef reverseActiveLow
  if (!isPinHigh(reversePin))
#else
  if (isPinHigh(reversePin))
#endif
  {
    drawReverse();
#ifdef backupBuzzer
    buzzerCount++;
    if (buzzerCount >= 65)
    {
      buzzer = (buzzer == LOW ? HIGH : LOW);
      buzzerCount = 0;
    }
#endif
  }
  // check for brakes
  else if (isPinHigh(brakePin))
  {
    pixels.fill(stopColor, 0, numLEDs);
#ifdef backupBuzzer
    buzzer = LOW;
    buzzerCount = 0;
#endif
  }
  // check for running lights
  else if (isPinHigh(lightsPin))
  {
    pixels.fill(lightsColor, 0, numLEDs);
  }
  // draw normal background
  else
  {
    pixels.fill(idleColor, 0, numLEDs);
#ifdef backupBuzzer
    buzzer = LOW;
    buzzerCount = 0;
#endif
  }

  // check for left turn
  if (isPinHigh(leftPin) || (leftPosition > 0 && leftPosition < numLEDs))
  {
    leftPosition++;
  #ifdef animateTurn
    int size = (leftPosition > numLEDs / 2) ? numLEDs / 2: leftPosition;
    pixels.fill(turnColor, numLEDs / 2 - size, size);
  #else
    pixels.fill(turnColor, 0, numLEDs / 2);
  #endif
  }
  else
  {
    leftPosition = 0;
  }
  // check for right turn
  if (isPinHigh(rightPin) || (rightPosition > 0 && rightPosition < numLEDs))
  {
    rightPosition++;
#ifdef animateTurn
    int size = (rightPosition > numLEDs / 2) ? numLEDs / 2: rightPosition;
    pixels.fill(turnColor, numLEDs / 2, size);
#else
    pixels.fill(turnColor, numLEDs / 2, numLEDs / 2);
#endif
  }
  else
  {
    rightPosition = 0;
  }

  // send pixels to LED strip
  pixels.show();
#ifdef backupBuzzer
  if (buzzer != previousBuzzer)
  {
    digitalWrite(buzzerPin, buzzer);
    previousBuzzer = buzzer;
  }
#endif
  delay(blinkRate/numLEDs);
}

void drawReverse()
{
  unsigned int color1 = reverseCount < reverseDelay ? reverseColor : idleColor;
  unsigned int color2 = reverseCount < reverseDelay ? idleColor : reverseColor;
  for (int position=0; position < numLEDs; position += reverseSize)
  {
    pixels.fill(position % (reverseSize*2) ? color1 : color2, position, reverseSize);
  }
  // pixels.fill(reverseColor, 0, numLEDs);
  reverseCount++;
  if (reverseCount > reverseDelay * 2)
  {
    reverseCount = 0;
  }
}

bool isPinHigh(int pin)
{
  bool result = (digitalRead(pin) == HIGH);
  return result;
}
