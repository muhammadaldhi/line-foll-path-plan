#include "servo.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace {
uint8_t clampAngle(int16_t angle) {
    if (angle < 0) return 0;
    if (angle > 180) return 180;
    return static_cast<uint8_t>(angle);
}
}

ServoController::ServoController(ServoWriteCallback write, int8_t leftPin,
                                 int8_t rightPin)
    : write_(write), leftPin_(leftPin), rightPin_(rightPin),
      leftAngle_(90), rightAngle_(90) {}

void ServoController::set(int8_t pin, uint8_t& state, int16_t angle) {
    state = clampAngle(angle);
    if (write_ && pin != -1) write_(pin, state);
}

void ServoController::setLeft(int16_t angle) {
    set(leftPin_, leftAngle_, angle);
}

void ServoController::setRight(int16_t angle) {
    set(rightPin_, rightAngle_, angle);
}

void ServoController::stop() {
    setLeft(90);
    setRight(90);
}

uint8_t ServoController::leftAngle() const {
    return leftAngle_;
}

uint8_t ServoController::rightAngle() const {
    return rightAngle_;
}
