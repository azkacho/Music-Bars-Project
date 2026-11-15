// src/display.h — antarmuka umum untuk semua jenis display.
// Implementasi dipilih saat kompilasi lewat build_flags di platformio.ini:
//   -DDISPLAY_SSD1306 / -DDISPLAY_SSD1305  -> display_oled.cpp
//   -DDISPLAY_LCD1602                      -> display_lcd.cpp
#pragma once
#include <Arduino.h>

constexpr uint8_t NUM_BARS = 16;

#if !defined(DISPLAY_SSD1306) && !defined(DISPLAY_SSD1305) && !defined(DISPLAY_LCD1602)
  #error "Pilih display: pilih environment di platformio.ini (DISPLAY_SSD1306 / DISPLAY_SSD1305 / DISPLAY_LCD1602)"
#endif

bool displayBegin();                        // false = display tidak terdeteksi
void displaySplash();                       // layar pembuka (nama display)
void displayWaiting();                      // layar "menunggu data PC"
void displayBarsStart();                    // dipanggil saat berpindah dari "menunggu" ke bar
void displayHandleInput();                  // baca tombol (jika ada), dipanggil tiap loop
void displayDrawBars(const float *level);   // level[NUM_BARS], tiap nilai 0.0 .. 1.0