<h2>Arduino IDE</h2>

Download IDE from https://www.arduino.cc/en/software

Under File > Preferences, set Additional boards manager URLs to https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

Add libraries:
 * Adafruit GFX by Adafruit
 * Adafruit NeoPixel by Adafruit
 * Adafruit SSD1306 by Adafruit

Under Tools > Board > Board Manager, add these boards
* STM32 MCU based boards by STMicroelectronics

Under Tools > Board, select STM32 boards > STM32F103CB (20k RAM, 128k Flash)

Connect ESP32 Development Kit with USB cable
Under Tools > Port, select COM port used

Open DistanceSensor.ino file and click right arrow on top tool bar to download code to ESP8266

