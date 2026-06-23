// include the libraries
#include "Adaline.h"
#include "Color.h"
#include <Adafruit_NeoTrellis.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <rgb_lcd.h>
#define LEDS NEO_TRELLIS_NUM_KEYS + 1

#if ESP32
const uint8_t SDA_PIN = 39;
const uint8_t SCL_PIN = 37;
const static uint8_t BTN_negative = 35;
const static uint8_t BTN_positive = 33;
#else
const static uint8_t BTN_positive = 2;
const static uint8_t BTN_negative = 3;
#endif

const static uint8_t OLED_COLS = 16;
const static uint8_t OLED_ROWS = 2;

TrellisCallback btnPress(keyEvent evt);
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_NeoTrellis trellis;
Adaline adaline = Adaline();
rgb_lcd lcd;

// 16 Neo Trellis Colors and 1 LCD Color
static Color source[LEDS];
static Color target[LEDS];
static uint16_t scalar[LEDS];
// Update the display when initializing
static boolean update_display = true;

void setup() {
  Serial.begin(115200);
  // Set the button pins as input with pull-up resistor.
  // They read HIGH when not pressed and LOW when pressed
  pinMode(BTN_positive, INPUT_PULLUP);
  pinMode(BTN_negative, INPUT_PULLUP);

#if ESP32
  // Lock I2C pins directly on the hardware.
  // Every library must now use pin 39 en 37
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  Wire.setClock(400000);
#endif

  // initialize NeoTrellis@0x2E
  if (!trellis.begin(0x2E)) {
    Serial.println("Could not start Trellis, check wiring?");
  } else {
    Serial.println("NeoPixel Trellis started");
    for (int i = 0; i < LEDS - 1; i++) {
      trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
      trellis.registerCallback(i, btnPress);
      target[i] = adaline.getInput(i) > 0 ? Color::POSITIVE : Color::NEGATIVE;
    }
  }
  // initialize the oled display
  if (!u8g2.begin())
    Serial.println("SSD1306 failed to initialize");
  else
    Serial.println("SSD1306 initialized");

  // initialize the lcd display
  lcd.begin(OLED_COLS, OLED_ROWS);
  Serial.println("LCD initialized");
}

// Callback function for the trellis, called when a buttons state changes
TrellisCallback btnPress(keyEvent evt) {
  uint16_t i = evt.bit.NUM;
  adaline.invertInput(i);
  source[i] = Color(scalar[i], source[i], target[i]);
  target[i] = adaline.getInput(i) > 0 ? Color::POSITIVE : Color::NEGATIVE;
  scalar[i] = 0;
  update_display = true;
  return 0;
}

void loop() {
  static unsigned long last_fading_ms = 0;
  static unsigned long last_button_ms = 0;

  if (millis() - last_fading_ms > 10) {
    last_fading_ms = millis();
    for (uint8_t i = 0; i < LEDS; i++)
      scalar[i] = min(scalar[i] + 10, 255);
    for (int i = 0; i < LEDS - 1; i++) {
      Color c = Color(scalar[i], source[i], target[i]);
      trellis.pixels.setPixelColor(i, c.bits() >> 8);
    }
    trellis.pixels.show();
    Color c = Color(scalar[LEDS - 1], source[LEDS - 1], target[LEDS - 1]);
    lcd.setRGB(c.r, c.g, c.b);
  }

  if (millis() - last_button_ms > 100) {
    last_button_ms = millis();
    // read the trellis and let it handle the callback
    trellis.read();
    if (!digitalRead(BTN_positive) && !digitalRead(BTN_negative)) {
      adaline.resetInputs();
      adaline.resetWeights();
      while (!digitalRead(BTN_positive) || !digitalRead(BTN_negative)) {
        delay(10);
      }
      for (int i = 0; i < LEDS - 1; i++) {
        source[i] = Color(scalar[i], source[i], target[i]);
        target[i] = adaline.getInput(i) > 0 ? Color::POSITIVE : Color::NEGATIVE;
        scalar[i] = 0;
      }
      update_display = true;
    } else if (!digitalRead(BTN_positive)) {
      adaline.makePositive();
      update_display = true;
    } else if (!digitalRead(BTN_negative)) {
      adaline.makeNegative();
      update_display = true;
    }
  }

  if (update_display) {
    // Update the lcd display with the current output
    double output = adaline.getOutput();
    double sigmoid = tanh(2 * output);
    lcd.setCursor(0, 0);
    lcd.print("output = ");
    lcd.print(output);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("result = ");
    lcd.print(sigmoid);
    lcd.print("  ");

    source[LEDS - 1] =
        Color(scalar[LEDS - 1], source[LEDS - 1], target[LEDS - 1]);
    scalar[LEDS - 1] = 0;
    if (sigmoid < 0)
      target[LEDS - 1] = Color(-255 * sigmoid, 0, 0);
    else
      target[LEDS - 1] = Color(0, 0, 255 * sigmoid);

    // Update the oled display with the current output
    u8g2.firstPage();
    do {
      char buffer[32];
      for (uint8_t i = 0; i < adaline.n; i++) {
        snprintf(buffer, sizeof(buffer), "%+.2f",
                 adaline.getWeight(i) * adaline.getInput(i));
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(i % 4 * 32, i / 4 * 10 + 10, buffer);
      }
      snprintf(buffer, sizeof(buffer), "Bias = %+.2f", adaline.getBias());
      u8g2.drawStr(0, 5 * 10 + 10, buffer);
    } while (u8g2.nextPage());
    update_display = false;
  }
}

// Scan the i2c bus to find all connected i2c devices
// Doesn't work with the RGB LCD because it is bugged
// Ask me how i know...
void scanI2C() {
  Wire.begin();
  // loop trough all the i2c addresses
  for (uint8_t i = 1; i < 127; i++) {
    // send a ping to the device
    Wire.beginTransmission(i);
    // if the device responds, print its address
    if (Wire.endTransmission() == 0) {
      Serial.print("0x");
      Serial.print(i, HEX);
      Serial.println(" ");
    }
  }
}