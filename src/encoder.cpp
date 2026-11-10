#include "encoder.h"

#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace {

int8_t transition(uint8_t previous, uint8_t current) {
    static constexpr int8_t table[4][4] = {
        {0, 1, -1, 0},
        {-1, 0, 0, 1},
        {1, 0, 0, -1},
        {0, -1, 1, 0}
    };
    return table[previous][current];
}

uint8_t state(bool a, bool b) {
    return static_cast<uint8_t>((static_cast<uint8_t>(a) << 1U) |
                                static_cast<uint8_t>(b));
}

} // namespace

EncoderController::EncoderController()
    : leftCount_(0), rightCount_(0), leftState_(0), rightState_(0) {
#ifdef ARDUINO
    if (ENC_L_A_PIN != -1 && ENC_L_B_PIN != -1) {
        pinMode(ENC_L_A_PIN, INPUT);
        pinMode(ENC_L_B_PIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(ENC_L_A_PIN), [] {}, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENC_L_B_PIN), [] {}, CHANGE);
    }
    if (ENC_R_A_PIN != -1 && ENC_R_B_PIN != -1) {
        pinMode(ENC_R_A_PIN, INPUT);
        pinMode(ENC_R_B_PIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(ENC_R_A_PIN), [] {}, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENC_R_B_PIN), [] {}, CHANGE);
    }
#endif
}

void EncoderController::updateLeft(bool a, bool b) {
    leftCount_ += transition(leftState_, state(a, b));
    leftState_ = state(a, b);
}

void EncoderController::updateRight(bool a, bool b) {
    rightCount_ += transition(rightState_, state(a, b));
    rightState_ = state(a, b);
}

int32_t EncoderController::leftCount() const { return leftCount_; }
int32_t EncoderController::rightCount() const { return rightCount_; }

void EncoderController::reset() {
    leftCount_ = 0;
    rightCount_ = 0;
}
