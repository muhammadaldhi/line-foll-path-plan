#include "buzzer.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

BuzzerController::BuzzerController(BuzzerCallback callback, int8_t pin)
    : callback_(callback), pin_(pin) {}

void BuzzerController::beep(uint16_t frequency, uint16_t durationMs) {
    lastEvent_ = {frequency, durationMs, true};
    if (pin_ == -1) {
        return;
    }
#ifdef ARDUINO
    tone(pin_, frequency, durationMs);
#endif
    if (callback_) {
        callback_(pin_, frequency, durationMs);
    }
}

void BuzzerController::stop() {
    lastEvent_ = {0, 0, false};
    if (pin_ == -1) {
        return;
    }
#ifdef ARDUINO
    noTone(pin_);
#endif
    if (callback_) {
        callback_(pin_, 0, 0);
    }
}

BuzzerEvent BuzzerController::lastEvent() const {
    return lastEvent_;
}
