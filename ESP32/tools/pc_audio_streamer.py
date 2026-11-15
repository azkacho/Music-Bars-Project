"""
pc_audio_streamer.py
Menangkap audio sistem laptop (loopback), menghitung spektrum 16 pita,
lalu mengirimnya ke ESP32 lewat USB Serial.

Instalasi : pip install numpy pyserial soundcard
Pemakaian : python pc_audio_streamer.py COM5
            python pc_audio_streamer.py /dev/ttyUSB0

Protokol (per frame, 17 byte):
    0xFF | b0 | b1 | ... | b15      (b = 0..254, 0xFF tidak pernah muncul di payload)
"""
import sys
import time

import numpy as np
import serial
import soundcard as sc

# ===================== KONFIGURASI =====================
BAUD = 115200
SAMPLE_RATE = 48000          # Hz
HOP = 1024                   # sampel baru per update (~21 ms)
N_FFT = 2048                 # panjang jendela FFT (resolusi ~23 Hz)
NUM_BARS = 16
F_MIN, F_MAX = 60.0, 12000.0
DB_FLOOR, DB_CEIL = -80.0, -25.0   # kalibrasi sesuai volume/lagu
TILT_DB_PER_BAR = 0.8
DEBUG = False                # True = cetak level dB maksimum tiap detik
# =======================================================


def make_band_edges():
    """Batas bin FFT untuk tiap bar (skala logaritmik)."""
    bin_w = SAMPLE_RATE / N_FFT
    edges = [max(1, int(round(F_MIN / bin_w)))]
    for i in range(1, NUM_BARS + 1):
        f = F_MIN * (F_MAX / F_MIN) ** (i / NUM_BARS)
        b = int(round(f / bin_w))
        if b <= edges[-1]:
            b = edges[-1] + 1
        edges.append(min(b, N_FFT // 2))
    return edges


def open_serial(port):
    """Buka port tanpa memicu auto-reset ESP32 (DTR/RTS dimatikan)."""
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = BAUD
    ser.dtr = False
    ser.rts = False
    ser.open()
    time.sleep(2.0)
    return ser


def main():
    if len(sys.argv) < 2:
        print("Pemakaian: python pc_audio_streamer.py <PORT>   (contoh: COM5)")
        sys.exit(1)

    ser = open_serial(sys.argv[1])
    edges = make_band_edges()
    window = np.hanning(N_FFT).astype(np.float32)
    win_sum = float(window.sum())
    tilt = TILT_DB_PER_BAR * np.arange(NUM_BARS)

    speaker = sc.default_speaker()
    mic = sc.get_microphone(id=str(speaker.name), include_loopback=True)
    print(f"Loopback dari : {speaker.name}")
    print(f"Kirim ke ESP32: {sys.argv[1]} @ {BAUD} baud")
    print("Tekan Ctrl+C untuk berhenti.")

    buf = np.zeros(N_FFT, dtype=np.float32)
    last_dbg = 0.0

    try:
        with mic.recorder(samplerate=SAMPLE_RATE) as rec:
            while True:
                data = rec.record(numframes=HOP)               # (HOP, channels)
                mono = data.mean(axis=1).astype(np.float32)

                buf = np.roll(buf, -HOP)
                buf[-HOP:] = mono

                spec = np.abs(np.fft.rfft(buf * window))
                amp = 2.0 * spec / win_sum                     # amplitudo sinus (0 dBFS = 1.0)

                db = np.empty(NUM_BARS, dtype=np.float32)
                for i in range(NUM_BARS):
                    m = amp[edges[i]:edges[i + 1]].max()
                    db[i] = 20.0 * np.log10(m + 1e-9)
                db += tilt

                norm = np.clip((db - DB_FLOOR) / (DB_CEIL - DB_FLOOR), 0.0, 1.0)
                payload = (norm * 254).astype(np.uint8)
                ser.write(b"\xff" + payload.tobytes())

                if DEBUG and time.time() - last_dbg > 1.0:
                    last_dbg = time.time()
                    print(f"maxDb: {db.max():.1f} dB  (floor {DB_FLOOR}, ceil {DB_CEIL})")
    except KeyboardInterrupt:
        print("\nBerhenti.")
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    finally:
        ser.close()


if __name__ == "__main__":
    main()