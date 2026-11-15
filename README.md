# Panduan Setup Pemilihan Layar — Music Bar Visualizer

Project ini bisa dijalankan di beberapa kombinasi **board** dan **layar** dengan kode yang sama. Layar dipilih lewat *environment* PlatformIO, tanpa mengubah kode.

| Board | Layar yang didukung |
|---|---|
| Arduino Uno | OLED SSD1306, OLED SSD1305, LCD 16x2 (I2C) |
| ESP32 DevKit V1 | OLED SSD1306, OLED SSD1305, LCD 16x2 (I2C) |

Data audio dikirim dari laptop lewat USB Serial oleh `pc_audio_streamer.py`. Panduan ini hanya membahas pemilihan layar dan pengunggahan firmware.

---

## 1. Pasang file-file project

1. Pindahkan `src/main.cpp` lama ke tempat lain sebagai cadangan, lalu hapus dari `src/`.
2. Salin file-file berikut ke project:
   - `platformio.ini` menimpa file di root project
   - `main.cpp`, `display.h`, `display_oled.cpp`, `display_lcd.cpp` ke dalam folder `src/`
3. Struktur akhir:

   ```
   OLED learn/
   ├── platformio.ini
   ├── src/
   │   ├── main.cpp            ← logika bersama (Serial, smoothing, status)
   │   ├── display.h           ← antarmuka umum display
   │   ├── display_oled.cpp    ← driver SSD1306 dan SSD1305
   │   └── display_lcd.cpp     ← driver LCD 16x2
   └── ESP32/tools/pc_audio_streamer.py   ← tidak berubah
   ```
4. Simpan semua file. PlatformIO akan memuat ulang konfigurasi, lalu pilihan environment muncul di status bar.

## 2. Tentukan environment

Nama environment berpola `<board>_<layar>`:

| Yang kamu pakai | Environment |
|---|---|
| Arduino Uno + LCD 16x2 | `uno_lcd` |
| Arduino Uno + OLED SSD1306 | `uno_ssd1306` |
| Arduino Uno + OLED SSD1305 | `uno_ssd1305` |
| ESP32 + LCD 16x2 | `esp32_lcd` |
| ESP32 + OLED SSD1306 | `esp32_ssd1306` |
| ESP32 + OLED SSD1305 | `esp32_ssd1305` |

## 3. Pilih environment di VS Code

**Lewat status bar:** klik tulisan `Default (OLED learn)` di bagian bawah VS Code, lalu pilih environment dari daftar yang muncul.

**Lewat Command Palette:** tekan `Ctrl+Shift+P`, jalankan `PlatformIO: Switch Project Environment`, lalu pilih environment.

Tombol **Build** (✓) dan **Upload** (→) hanya berlaku untuk environment yang dipilih. Jika pilihan tetap `Default`, yang dipakai adalah `default_envs` di bagian `[platformio]` pada `platformio.ini` (bawaan: `uno_lcd`). Ubah baris itu jika ingin default lain. Tanpa `default_envs`, Build akan mencoba membangun semua environment sekaligus.

Lewat terminal:

```
pio run -e uno_lcd              # build
pio run -e uno_lcd -t upload    # upload
```

## 4. Build, upload, dan cek

1. Klik **Build**. Build pertama untuk tiap environment mengunduh library-nya, jadi butuh internet dan agak lama.
2. Colok board, lalu cek port:
   ```
   pio device list
   ```
   Nomor COM Uno dan ESP32 bisa berbeda.
3. Tutup Serial Monitor dan skrip Python jika sedang terbuka, lalu klik **Upload**.
4. Setelah upload, layar menampilkan:

   | Layar | Splash (1,2 detik) | Lalu |
   |---|---|---|
   | OLED | "Music Bars / OLED SSD1306" (atau SSD1305) | "Menunggu data PC..." |
   | LCD | "Music Bars / LCD 16x2" | "Menunggu data PC / via USB Serial" |

5. Jika layar tidak menampilkan apa-apa, buka Serial Monitor (115200). Firmware mencetak `Display tidak ditemukan!` bila layar tidak terdeteksi.

## 5. Jalankan streaming dari laptop

Dari terminal dengan virtual environment aktif `(.venv)`, di folder `ESP32\tools`:

```
python pc_audio_streamer.py COM11
```

Ganti `COM11` dengan port board yang sedang terhubung, lalu putar musik di laptop.

## 6. Berganti layar atau board

1. Hentikan skrip Python (`Ctrl+C`) dan tutup Serial Monitor.
2. Pilih environment lain (langkah 3).
3. Pasang board dan layar yang sesuai, atur wiring (tabel di bawah), lalu Build dan Upload.

Tiap environment punya folder build sendiri di `.pio\build\<nama_env>`, jadi tidak perlu Clean setiap kali berganti.

## Wiring I2C

| Layar | VCC | SDA | SCL |
|---|---|---|---|
| OLED (Uno) | 5V atau 3,3V | A4 | A5 |
| OLED (ESP32) | 3,3V | GPIO21 | GPIO22 |
| LCD I2C (Uno) | 5V | A4 | A5 |
| LCD I2C (ESP32) | VIN (5V) | GPIO21 | GPIO22 |

GND layar ke GND board. Tombol mode hanya dipakai OLED: D2 ke GND pada Uno, atau GPIO27 ke GND pada ESP32.

---
