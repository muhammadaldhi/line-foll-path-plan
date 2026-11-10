# Auxiliary Hardware Design

## Intent

Sediakan API modular untuk servo, encoder, dan buzzer ESP32-C3. Fungsi tetap tersedia walau pin masih `-1`, sehingga planner dan host test bisa berjalan tanpa hardware fisik.

## Goals

- Servo kiri/kanan menerima command posisi.
- Encoder kiri/kanan menghitung quadrature melalui API software.
- Buzzer menerima event beep.
- Pin `-1` melewati operasi hardware tanpa crash.
- Semua modul host-testable dan bebas coupling ke planner.

## Non-goals

- Mengubah logika planner.
- Menetapkan ulang pin ESP32-C3.
- Menambahkan aksesoris ke storage sebelum action runtime memakainya.

## Architecture

Modul terpisah:

```text
servo.h/.cpp
encoder.h/.cpp
buzzer.h/.cpp
```

Hardware callback atau blok `#ifdef ARDUINO` menangani output fisik. State dan command tetap bisa diamati pada host.

API proposal:

```cpp
ServoController::setLeft(uint8_t angle)
ServoController::setRight(uint8_t angle)
ServoController::stop()

EncoderController::updateLeft(bool a, bool b)
EncoderController::updateRight(bool a, bool b)
EncoderController::leftCount()
EncoderController::rightCount()
EncoderController::reset()

BuzzerController::beep(uint16_t frequency, uint16_t durationMs)
BuzzerController::stop()
```

## Decisions

- Servo angle dibatasi `0..180`.
- Encoder count bertipe signed agar arah bisa negatif.
- Quadrature transition invalid diabaikan.
- Buzzer pin `-1` tetap mencatat event terakhir, tetapi tidak memanggil GPIO/PWM.
- Callback hardware di-inject agar test host tidak membutuhkan Arduino.

## Remaining

Implementer menentukan detail callback minimal, validasi transition quadrature, dan representasi event buzzer tanpa mengubah keputusan di atas.
