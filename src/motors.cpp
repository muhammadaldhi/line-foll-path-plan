#include "motors.h"
#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace {
constexpr int16_t MOTOR_LIMIT = 255;
}

MotorController::MotorController(MotorWriteCallback write, MotorPwmCallback pwm)
    : write_(write), pwm_(pwm) {}

uint8_t MotorController::magnitude(int16_t speed) {
    if (speed > MOTOR_LIMIT) speed = MOTOR_LIMIT;
    if (speed < -MOTOR_LIMIT) speed = -MOTOR_LIMIT;
    return static_cast<uint8_t>(speed < 0 ? -speed : speed);
}

void MotorController::driveMotor(int8_t forwardPin, int8_t reversePin, int16_t speed) {
    if (speed > MOTOR_LIMIT) speed = MOTOR_LIMIT;
    if (speed < -MOTOR_LIMIT) speed = -MOTOR_LIMIT;

    const bool forward = speed >= 0;
    const int8_t pwmPin = forward ? forwardPin : reversePin;
    const int8_t lowPin = forward ? reversePin : forwardPin;
    if (write_ && lowPin != -1) write_(lowPin, 0);
    if (pwm_ && pwmPin != -1) pwm_(pwmPin, magnitude(speed));
}

void MotorController::drive(int16_t rightSpeed, int16_t leftSpeed) {
    driveMotor(PWMR2, PWMR1, rightSpeed);
    driveMotor(PWML1, PWML2, leftSpeed);
}

void MotorController::stop() {
    if (write_) {
        if (PWMR1 != -1) write_(PWMR1, 0);
        if (PWMR2 != -1) write_(PWMR2, 0);
        if (PWML1 != -1) write_(PWML1, 0);
        if (PWML2 != -1) write_(PWML2, 0);
    }
    if (pwm_) {
        if (PWMR1 != -1) pwm_(PWMR1, 0);
        if (PWMR2 != -1) pwm_(PWMR2, 0);
        if (PWML1 != -1) pwm_(PWML1, 0);
        if (PWML2 != -1) pwm_(PWML2, 0);
    }
}

#ifdef ARDUINO
void initializeMotors() {
    pinMode(PWMR1, OUTPUT);
    pinMode(PWMR2, OUTPUT);
    pinMode(PWML1, OUTPUT);
    pinMode(PWML2, OUTPUT);
}

MotorController& hardwareMotors() {
    static MotorController controller(
        [](int8_t pin, uint8_t value) { digitalWrite(pin, value ? HIGH : LOW); },
        [](int8_t pin, uint16_t value) { analogWrite(pin, value); });
    return controller;
}
#endif
