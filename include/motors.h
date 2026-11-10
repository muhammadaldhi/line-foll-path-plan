#ifndef MOTORS_H
#define MOTORS_H

#include <cstdint>
#include <functional>

using MotorWriteCallback = std::function<void(int8_t, uint8_t)>;
using MotorPwmCallback = std::function<void(int8_t, uint16_t)>;

class MotorController {
public:
    explicit MotorController(MotorWriteCallback write, MotorPwmCallback pwm);

    void drive(int16_t rightSpeed, int16_t leftSpeed);
    void stop();

private:
    void driveMotor(int8_t forwardPin, int8_t reversePin, int16_t speed);
    static uint8_t magnitude(int16_t speed);

    MotorWriteCallback write_;
    MotorPwmCallback pwm_;
};

#ifdef ARDUINO
void initializeMotors();
MotorController& hardwareMotors();
#endif

#endif
