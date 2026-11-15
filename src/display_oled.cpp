// src/display_oled.cpp — driver OLED 128x64 I2C (SSD1306 atau SSD1305)
// Aktif jika build_flags berisi -DDISPLAY_SSD1306 atau -DDISPLAY_SSD1305
#if defined(DISPLAY_SSD1306) || defined(DISPLAY_SSD1305)

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "display.h"

#if defined(DISPLAY_SSD1306)
  #include <Adafruit_SSD1306.h>
  #define OLED_NAME "SSD1306"
  static Adafruit_SSD1306 oled(128, 64, &Wire, -1, 400000UL, 400000UL);
  static bool oledBegin() { return oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); }
#else
  #include <Adafruit_SSD1305.h>
  #define OLED_NAME "SSD1305"
  static Adafruit_SSD1305 oled(128, 64, &Wire, -1, 400000UL, 400000UL);
  static bool oledBegin() { return oled.begin(0x3C); }
#endif

// I2C: ESP32 -> SDA=GPIO21, SCL=GPIO22 | Uno -> SDA=A4, SCL=A5
#if defined(ESP32)
  constexpr uint8_t PIN_BTN = 27;   // tombol ganti mode (ke GND)
#else
  constexpr uint8_t PIN_BTN = 2;
#endif

constexpr int      SCREEN_W     = 128;
constexpr int      SCREEN_H     = 64;
constexpr uint16_t PIXEL_ON     = 1;
constexpr int      BAR_W        = SCREEN_W / NUM_BARS;   // 8 px
constexpr float    PEAK_FALL    = 0.02f;
constexpr uint32_t PEAK_HOLD_MS = 350;

static float    peak[NUM_BARS]     = {0};
static uint32_t peakTime[NUM_BARS] = {0};
static uint8_t  mode = 0;           // 0 = bars + peak, 1 = mirror

bool displayBegin() {
  pinMode(PIN_BTN, INPUT_PULLUP);
  Wire.begin();
  if (!oledBegin()) return false;
  oled.setTextSize(1);
  oled.setTextColor(PIXEL_ON);
  return true;
}

void displaySplash() {
  oled.clearDisplay();
  oled.setCursor(25, 22);
  oled.print(F("Music Bars"));
  oled.setCursor(25, 36);
  oled.print(F("OLED " OLED_NAME));
  oled.display();
}

void displayWaiting() {
  oled.clearDisplay();
  oled.setCursor(7, 28);
  oled.print(F("Menunggu data PC..."));
  oled.display();
}

void displayBarsStart() {
  for (uint8_t i = 0; i < NUM_BARS; i++) peak[i] = 0;
}

void displayHandleInput() {
  static bool lastState = HIGH;
  static uint32_t lastPress = 0;
  bool s = digitalRead(PIN_BTN);
  if (lastState == HIGH && s == LOW && millis() - lastPress > 200) {
    mode = (mode + 1) % 2;
    lastPress = millis();
  }
  lastState = s;
}

void displayDrawBars(const float *level) {
  uint32_t now = millis();
  oled.clearDisplay();

  if (mode == 0) {
    for (uint8_t i = 0; i < NUM_BARS; i++) {
      // peak hold
      if (level[i] >= peak[i]) {
        peak[i] = level[i];
        peakTime[i] = now;
      } else if (now - peakTime[i] > PEAK_HOLD_MS) {
        peak[i] = max(level[i], peak[i] - PEAK_FALL);
      }

      int x = i * BAR_W;
      int h = (int)(level[i] * SCREEN_H);
      if (h > 0) oled.fillRect(x, SCREEN_H - h, BAR_W - 1, h, PIXEL_ON);
      if (peak[i] > 0.02f) {
        int py = SCREEN_H - 1 - (int)(peak[i] * (SCREEN_H - 1));
        oled.drawFastHLine(x, py, BAR_W - 1, PIXEL_ON);
      }
    }
  } else {
    const int half = SCREEN_H / 2;
    for (uint8_t i = 0; i < NUM_BARS; i++) {
      int x = i * BAR_W;
      int h = (int)(level[i] * half);
      if (h < 1) h = 1;
      oled.fillRect(x, half - h, BAR_W - 1, h * 2, PIXEL_ON);
    }
  }

  oled.display();
}

#endif  // DISPLAY_SSD1306 || DISPLAY_SSD1305