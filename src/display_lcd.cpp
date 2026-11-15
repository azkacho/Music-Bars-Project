// src/display_lcd.cpp — driver LCD 16x2 dengan backpack I2C (PCF8574)
// Aktif jika build_flags berisi -DDISPLAY_LCD1602
// I2C: ESP32 -> SDA=GPIO21, SCL=GPIO22 | Uno -> SDA=A4, SCL=A5
#if defined(DISPLAY_LCD1602)

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "display.h"

constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;

static hd44780_I2Cexp lcd;                          // alamat & pin backpack terdeteksi otomatis
static uint8_t prevCell[LCD_ROWS][LCD_COLS];        // cache isi LCD, tulis hanya jika berubah

// Karakter custom: slot 0..7 = blok setinggi 1..8 baris piksel
static void createGlyphs() {
  for (uint8_t h = 1; h <= 8; h++) {
    uint8_t glyph[8];
    for (uint8_t r = 0; r < 8; r++) {
      glyph[r] = (r >= 8 - h) ? 0x1F : 0x00;
    }
    lcd.createChar(h - 1, glyph);
  }
}

bool displayBegin() {
  if (lcd.begin(LCD_COLS, LCD_ROWS)) return false;  // != 0 berarti gagal
  createGlyphs();
  return true;
}

void displaySplash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Music Bars"));
  lcd.setCursor(0, 1);
  lcd.print(F("LCD 16x2"));
}

void displayWaiting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Menunggu data PC"));
  lcd.setCursor(0, 1);
  lcd.print(F("via USB Serial"));
}

void displayBarsStart() {
  lcd.clear();
  memset(prevCell, 0xFF, sizeof(prevCell));          // paksa tulis ulang semua sel
}

void displayHandleInput() {
  // LCD tidak memakai tombol
}

void displayDrawBars(const float *level) {
  uint8_t cells[LCD_ROWS][LCD_COLS];

  for (uint8_t i = 0; i < NUM_BARS; i++) {
    int total  = (int)(level[i] * 16.0f + 0.5f);     // 0..16
    int bottom = total > 8 ? 8 : total;              // baris bawah (1)
    int top    = total - 8;                          // baris atas  (0)

    cells[1][i] = (bottom <= 0) ? ' ' : (uint8_t)(bottom - 1);
    cells[0][i] = (top    <= 0) ? ' ' : (uint8_t)(top - 1);
  }

  for (uint8_t r = 0; r < LCD_ROWS; r++) {
    if (memcmp(cells[r], prevCell[r], LCD_COLS) == 0) continue;
    lcd.setCursor(0, r);
    for (uint8_t i = 0; i < LCD_COLS; i++) lcd.write(cells[r][i]);
    memcpy(prevCell[r], cells[r], LCD_COLS);
  }
}

#endif  // DISPLAY_LCD1602