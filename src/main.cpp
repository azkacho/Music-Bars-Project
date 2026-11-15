// src/main.cpp — Music Bar Visualizer (logika bersama untuk semua display)
// Data dari laptop via USB Serial. Frame: 0xFF + 16 byte (nilai 0..254 per bar)
// Jenis display dipilih lewat environment PlatformIO (lihat platformio.ini).
#include <Arduino.h>
#include "display.h"

// ===================== PROTOKOL & ANIMASI =====================
constexpr uint8_t  FRAME_HEADER = 0xFF;
constexpr float    DECAY        = 0.07f;      // kecepatan bar turun per frame
constexpr uint32_t TIMEOUT_MS   = 1000;       // tanpa data > 1 s = dianggap terputus

static float    target[NUM_BARS] = {0};
static float    level[NUM_BARS]  = {0};

static uint8_t  rxBuf[NUM_BARS];
static uint8_t  rxCount = 0;
static bool     inFrame = false;
static uint32_t lastFrameMs = 0;
static bool     everConnected = false;
static bool     barsMode = false;             // false = layar "menunggu"

// ---------- Terima data dari PC ----------
void readSerial() {
  while (Serial.available()) {
    uint8_t b = Serial.read();
    if (b == FRAME_HEADER) {                  // awal frame (juga resync bila ada byte hilang)
      inFrame = true;
      rxCount = 0;
      continue;
    }
    if (!inFrame) continue;

    rxBuf[rxCount++] = b;
    if (rxCount == NUM_BARS) {
      for (uint8_t i = 0; i < NUM_BARS; i++) {
        float v = rxBuf[i] / 254.0f;
        target[i] = v > 1.0f ? 1.0f : v;
      }
      lastFrameMs = millis();
      everConnected = true;
      inFrame = false;
      rxCount = 0;
    }
  }
}

// ---------- Smoothing: naik instan, turun perlahan ----------
void updateLevels(bool connected) {
  for (uint8_t i = 0; i < NUM_BARS; i++) {
    float t = connected ? target[i] : 0.0f;
    if (t >= level[i]) level[i] = t;
    else               level[i] = max(t, level[i] - DECAY);
  }
}

bool allLevelsZero() {
  for (uint8_t i = 0; i < NUM_BARS; i++) {
    if (level[i] > 0.001f) return false;
  }
  return true;
}

// ---------- Arduino ----------
void setup() {
#if defined(ESP32)
  Serial.setRxBufferSize(1024);
#endif
  Serial.begin(115200);

  if (!displayBegin()) {
    Serial.println(F("Display tidak ditemukan! Cek wiring, daya, dan pilihan environment."));
    while (true) delay(1000);
  }
  displaySplash();
  delay(1200);
  displayWaiting();
}

void loop() {
  readSerial();
  displayHandleInput();

  bool connected = everConnected && (millis() - lastFrameMs < TIMEOUT_MS);

  if (!barsMode) {
    if (!connected) { delay(5); return; }     // masih menunggu data pertama
    displayBarsStart();
    barsMode = true;
  }

  updateLevels(connected);
  displayDrawBars(level);

  if (!connected && allLevelsZero()) {        // data berhenti & bar sudah habis
    displayWaiting();
    barsMode = false;
  }
}