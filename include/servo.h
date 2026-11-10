#ifndef SERVO_H
#define SERVO_H

#include <cstdint>
#include <functional>

#include "config.h"

using ServoWriteCallback = std::function<void(int8_t, uint8_t)>;

class ServoController {
public:
    ServoController(ServoWriteCallback write, int8_t leftPin = SERVO_L_PIN,
                    int8_t rightPin = SERVO_R_PIN);

    void setLeft(int16_t angle);
    void setRight(int16_t angle);
    void stop();
    uint8_t leftAngle() const;
    uint8_t rightAngle() const;

private:
    void set(int8_t pin, uint8_t& state, int16_t angle);

    ServoWriteCallback write_;
    int8_t leftPin_;
    int8_t rightPin_;
    uint8_t leftAngle_;
    uint8_t rightAngle_;
};

#endif
