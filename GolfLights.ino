#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "golf_lights_html.h"

// config for wiring harness
#define leftPin       12
#define rightPin      13
#define reversePin    14
#define brakePin       4
#define reverseActive isPinHigh(reversePin) // for Yamaha
// #define reverseActive !isPinHigh(reversePin)   // for Star EV

// config for LED strip
#define ledPin        D1
#define numLEDs       48  // number of LEDs used in strip, needs to be an even number
#define maxBright     255 // 0-255 max brightness; to prevent overcurrent, start low
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);
float pixelSize = 1.0;    // the physical distance that each LED represents in cm
int blinkRate = 300;
unsigned long idleColor;
unsigned long stopColor;
unsigned long turnColor;
unsigned long reverseColor;

// WiFi config
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255,0);
ESP8266WebServer server(80);

// general config
unsigned int leftPosition = 0;
unsigned int rightPosition = 0;

// Arduino setup function. Runs in CPU 1
void setup()
{
  Serial.begin(74880);
  Serial.printf("\n\nStarting golf lights  LED: %d\n", numLEDs);

  EEPROM.begin(sizeof(idleColor) * 4);
  EEPROM.get(0, idleColor);
  EEPROM.get(4, stopColor);
  EEPROM.get(8, turnColor);
  EEPROM.get(12, reverseColor);
  idleColor &= 0x00ffffff; // mask to 3 bytes
  stopColor &= 0x00ffffff;
  turnColor &= 0x00ffffff;
  reverseColor &= 0x00ffffff;
  Serial.printf("idle: %06lx  stop: %06lx  turn: %06lx  reverse: %06lx\n", idleColor, stopColor, turnColor, reverseColor);
  storeColor(index_html, "running", idleColor);
  storeColor(index_html, "brakes", stopColor);
  storeColor(index_html, "turn", turnColor);
  storeColor(index_html, "reverse", reverseColor);

  // set up wiring harness
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
  #ifdef activeLowReverse
  pinMode(reversePin, INPUT_PULLUP); // active low
  #else
  pinMode(reversePin, INPUT);
  #endif
  pinMode(brakePin, INPUT);

  // set up LED strip
  pixels.begin();
  pixels.setBrightness(maxBright);
  // runningLEDs();
  pixels.fill(idleColor, 0, pixels.numPixels());
  pixels.show();

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
  
}

// Arduino loop function. Runs in CPU 1.
void loop()
{
  processPixels();
  server.handleClient();
}

void processPixels()
{
  unsigned long backColor = idleColor;
  // default
  pixels.fill(idleColor, 0, pixels.numPixels());
  // check for brakes
  if (isPinHigh(brakePin))
  {
    pixels.fill(stopColor, 0, pixels.numPixels());
    backColor = stopColor;
  }
  // check for reverse
  if (reverseActive)
  {
    pixels.fill(reverseColor, 0, pixels.numPixels());
    backColor = reverseColor;
  }
  // check for left turn
  if (isPinHigh(leftPin) || (leftPosition > 0 && leftPosition <= pixels.numPixels()))
  {
    for (int position = 0; position < pixels.numPixels()/2; position++)
    {
      pixels.setPixelColor(pixels.numPixels()/2 - position - 1, (position <= leftPosition ? turnColor : backColor));
    }
    leftPosition++;
  }
  else
  {
    leftPosition = 0;
  }
  // check for right turn
  if (isPinHigh(rightPin) || (rightPosition > 0 && rightPosition <= pixels.numPixels()))
  {
    for (int position = 0; position < pixels.numPixels()/2; position++)
    {
      pixels.setPixelColor(pixels.numPixels()/2 + position, (position <= rightPosition ? turnColor : backColor));
    }
    rightPosition++;
  }
  else
  {
    rightPosition = 0;
  }
  pixels.show();
  delay(blinkRate/pixels.numPixels());
}

bool isPinHigh(int pin)
{
  bool result = (digitalRead(pin) == HIGH);
  return result;
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
  Serial.printf("idle: %06lx  stop: %06lx  turn: %06lx  reverse: %06lx\n", idleColor, stopColor, turnColor, reverseColor);
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
