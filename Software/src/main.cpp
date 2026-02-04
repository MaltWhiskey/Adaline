// include the libraries
#include "Adaline.h"
#include <Adafruit_NeoTrellis.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <rgb_lcd.h>

const static uint8_t BTN_positive = 2;
const static uint8_t BTN_negative = 3;
const static uint8_t OLED_COLS = 16;
const static uint8_t OLED_ROWS = 2;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
rgb_lcd lcd;
Adafruit_NeoTrellis trellis;
TrellisCallback btnPress(keyEvent evt);
void updateDisplay();
void scanI2C();
Adaline adaline = Adaline();

void setup() {
  Serial.begin(115200);
  // Set the button pins as input with pull-up resistor.
  // They read HIGH when not pressed and LOW when pressed
  pinMode(BTN_positive, INPUT_PULLUP);
  pinMode(BTN_negative, INPUT_PULLUP);

  // initialize NeoTrellis@0x2E
  if (!trellis.begin(0x2E)) {
    Serial.println("Could not start Trellis, check wiring?");
  } else {
    Serial.println("NeoPixel Trellis started");
  }
  for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, btnPress);
  }

  // initialize the oled display
  if (!u8g2.begin())
    Serial.println("SSD1306 failed to initialize");
  else
    Serial.println("SSD1306 initialized");

  // initialize the lcd display
  lcd.begin(OLED_COLS, OLED_ROWS);
  Serial.println("LCD initialized");

  // display starting information
  updateDisplay();
}

// Callback function for the trellis, called when a buttons state changes
TrellisCallback btnPress(keyEvent evt) {
  adaline.invertInput(evt.bit.NUM);
  return 0;
}

void loop() {
  // read the trellis and let it handle the callback
  trellis.read();
  // update the leds to reflect the current state
  for (int i = 0; i < adaline.n; i++) {
    trellis.pixels.setPixelColor(i, adaline.getInput(i) > 0 ? 0x000015 : 0x150000);
    trellis.pixels.show();
  }

  if (!digitalRead(BTN_positive) || !digitalRead(BTN_negative)) {
    if (!digitalRead(BTN_positive) && !digitalRead(BTN_negative)) {
      adaline.resetInputs();
      adaline.resetWeights();
    } else if (!digitalRead(BTN_positive)) {
      adaline.makePositive();
    } else if (!digitalRead(BTN_negative)) {
      adaline.makeNegative();
    }
    updateDisplay();
    delay(50);
  }
}

void updateDisplay() {
  // Update the lcd display with the current output
  double output = adaline.getOutput();
  double sigmoid = tanh(2 * output);
  lcd.setCursor(0, 0);
  lcd.print("output = ");
  lcd.print(output);
  lcd.setCursor(0, 1);
  lcd.print("f(output) = ");
  lcd.print(sigmoid);
  if (output < 0)
    lcd.setRGB(-255 * sigmoid, 0, 0);
  else
    lcd.setRGB(0, 0, 255 * sigmoid);

  // Update the oled display with the current output
  u8g2.firstPage();
  do {
    char buffer[32];
    for (uint8_t i = 0; i < adaline.n; i++) {
      snprintf(buffer, sizeof(buffer), "%+.2f", adaline.getWeight(i) * adaline.getInput(i));
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(i / 4 * 32, i % 4 * 10 + 10, buffer);
    }
    snprintf(buffer, sizeof(buffer), "Bias = %+.2f", adaline.getBias());
    u8g2.drawStr(0, 5 * 10 + 10, buffer);
  } while (u8g2.nextPage());
}

// To find all connected i2c devices
// You can scan the i2c bus for devices
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