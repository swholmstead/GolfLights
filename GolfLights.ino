// CONFIG PARAMETERS
// #define runningIdle 1 // add a running black pixel on idle
// #define christmasIdle 1 // Christmas themed background
// #define reverseActiveLow 1 // for Star EV, comment out for Yamaha
// #define wifiEnabled 1  // add web page to change colors
// #define highDefLed 1 // 144 pixels/m

#include <Adafruit_NeoPixel.h>
#ifdef wifiEnabled
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "golf_lights_html.h"
#endif

// config for wiring harness
#define leftPin       D6
#define rightPin      D7
#define buzzerPin     D1
#define reversePin    D5
#define brakePin      D2

// config for LED strip
#define ledPin        D3
#ifdef highDefLed
#define numLEDs      116  // number of LEDs used in strip, needs to be an even number
#define maxBright    127 // 0-255 max brightness; to prevent overcurrent, start low
#else
#define numLEDs       44
#define maxBright    255 // 0-255 max brightness; to prevent overcurrent, start low
#endif
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);
int blinkRate = 300;
unsigned long idleColor;
unsigned long stopColor;
unsigned long turnColor;
unsigned long reverseColor;

#ifdef wifiEnabled
// WiFi config
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255,0);
ESP8266WebServer server(80);
#endif

// general config
unsigned int leftPosition = 0;
unsigned int rightPosition = 0;
unsigned int idlePosition = 0;
int idleDirection = 1;

// Arduino setup function. Runs in CPU 1
void setup()
{
  Serial.begin(74880);
  Serial.printf("\n\nStarting golf lights  LED: %d\n", numLEDs);

#ifdef wifiEnabled
  EEPROM.begin(sizeof(idleColor) * 4);
  EEPROM.get(0, idleColor);
  EEPROM.get(4, stopColor);
  EEPROM.get(8, turnColor);
  EEPROM.get(12, reverseColor);
  idleColor    &= 0x00ffffff; // mask to 3 bytes
  stopColor    &= 0x00ffffff;
  turnColor    &= 0x00ffffff;
  reverseColor &= 0x00ffffff;
  storeColor(index_html, "running", idleColor);
  storeColor(index_html, "brakes", stopColor);
  storeColor(index_html, "turn", turnColor);
  storeColor(index_html, "reverse", reverseColor);
#else
  idleColor = 0x000080;
  stopColor = 0xff0000;
  turnColor = 0xff7f00;
  reverseColor = 0xffffff;
#endif
  Serial.printf("idle: %06lx  stop: %06lx  turn: %06lx  reverse: %06lx\n", idleColor, stopColor, turnColor, reverseColor);

  // set up wiring harness
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
#ifdef reverseActiveLow
  pinMode(reversePin, INPUT_PULLUP); // active low
#else
  pinMode(reversePin, INPUT);
#endif
  pinMode(brakePin, INPUT);

  // set up LED strip
  pixels.begin();
  pixels.setBrightness(maxBright);
  pixels.fill(idleColor, 0, pixels.numPixels());
  pixels.show();

#ifdef wifiEnabled
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP("GolfCart") && WiFi.softAPConfig(local_IP, gateway, subnet))
  {
    Serial.printf("Access Point started, SSID: %s IP: ", WiFi.softAPSSID());
    Serial.println(WiFi.softAPIP());
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();
  }
  else
  {
    Serial.println("AP Config failed.");
  }
#endif
}

// Arduino loop function. Runs in CPU 1.
void loop()
{
  processPixels();
#ifdef wifiEnabled
  server.handleClient();
#endif
}

void processPixels()
{
  uint8_t turning = LOW;
  drawBackground();

  // check for brakes
  if (isPinHigh(brakePin))
  {
    pixels.fill(stopColor, 0, pixels.numPixels());
  }
  // check for reverse
#ifdef reverseActiveLow
  if (!isPinHigh(reversePin))
#else
  if (isPinHigh(reversePin))
#endif
  {
    pixels.fill(reverseColor, 0, pixels.numPixels());
  }
  // check for left turn
  if (isPinHigh(leftPin) || (leftPosition > 0 && leftPosition <= pixels.numPixels()))
  {
    leftPosition++;
    int size = leftPosition > pixels.numPixels() / 2 ? pixels.numPixels() / 2 : leftPosition;
    pixels.fill(turnColor, pixels.numPixels() / 2 - size, size);

    // limit amount of time buzzer is on
    if (leftPosition < pixels.numPixels() / 3)
    {
      turning = HIGH;
    }
  }
  else
  {
    leftPosition = 0;
  }
  // check for right turn
  if (isPinHigh(rightPin) || (rightPosition > 0 && rightPosition <= pixels.numPixels()))
  {
    rightPosition++;
    int size = rightPosition > pixels.numPixels() / 2 ? pixels.numPixels() / 2 : rightPosition;
    pixels.fill(turnColor, pixels.numPixels() / 2, size);

    // limit amount of time buzzer is on
    if (rightPosition < pixels.numPixels() / 3)
    {
      turning = HIGH;
    }
  }
  else
  {
    rightPosition = 0;
  }
  pixels.show();
  digitalWrite(buzzerPin, turning);
  delay(blinkRate/pixels.numPixels());
}

void drawBackground()
{
  // default
#ifndef christmasIdle
  pixels.fill(idleColor, 0, pixels.numPixels());
#endif
#ifdef runningIdle
#ifdef highDefLed
  pixels.fill(0xff0000, idlePosition / 2, 4);
#else
  pixels.fill(0xff0000, idlePosition / 2, 2);
#endif
  idlePosition += idleDirection;
  if (idlePosition <= 0 || idlePosition >= pixels.numPixels() * 2 - 1)
    idleDirection *= -1;
#endif
#ifdef christmasIdle
  for (int idlePosition = 0; idlePosition < pixels.numPixels(); idlePosition++)
  {
#ifdef highDefLed
    switch (idlePosition / 4 % 2)
#else
    switch (idlePosition / 2 % 2)
#endif
    {
      case 0:
        pixels.setPixelColor(idlePosition, 0x1f0000);
        break;
      case 1:
        pixels.setPixelColor(idlePosition, 0x001f00);
        break;
    }
  }
#endif
}

bool isPinHigh(int pin)
{
  bool result = (digitalRead(pin) == HIGH);
  return result;
}

#ifdef wifiEnabled
void handleRoot()
{
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html", index_html.c_str());
}

void handleSave()
{
  if (server.method() != HTTP_POST)
  {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String body = server.arg("plain");
  if (body.indexOf("running") > 0)
    idleColor = extractHex(body, "running");
  if (body.indexOf("brakes") > 0)
    stopColor = extractHex(body, "brakes");
  if (body.indexOf("turn") > 0)
    turnColor = extractHex(body, "turn");
  if (body.indexOf("reverse") > 0)
    reverseColor = extractHex(body, "reverse");
  EEPROM.put(0, idleColor);
  EEPROM.put(4, stopColor);
  EEPROM.put(8, turnColor);
  EEPROM.put(12, reverseColor);
  EEPROM.commit();
  storeColor(index_html, "running", idleColor);
  storeColor(index_html, "brakes", stopColor);
  storeColor(index_html, "turn", turnColor);
  storeColor(index_html, "reverse", reverseColor);
  // Serial.printf("idle: %06lx  stop: %06lx  turn: %06lx  reverse: %06lx\n", idleColor, stopColor, turnColor, reverseColor);
}

unsigned long extractHex(const String& json, const String& key)
{
  int start = json.indexOf("\"" + key + "\":\"#");
  if (start == -1)
    return 0x000000;
  start += key.length() + 5;
  String result = json.substring(start, start + 7);
  if (result.startsWith("#"))
    result.remove(0, 1);

  return strtoul(result.c_str(), nullptr, 16);
}

void storeColor(String& json, const String& key, unsigned long color)
{
  int start = json.indexOf("id=\"" + key + "\"");
  if (start == -1)
    return;
  start += 14 + key.length();
  json.remove(start, 6);
  String hexColor = "00000000" + String(color, HEX);
  hexColor = hexColor.substring(hexColor.length() - 6);
  json = json.substring(0, start) + hexColor + json.substring(start);
}
#endif
