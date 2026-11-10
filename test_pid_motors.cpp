#include <cassert>
#include <cstdint>
#include <vector>
#include "motors.h"
#include "pid.h"

struct Signal { int8_t pin; uint16_t value; };
static std::vector<Signal> signals;

static void writePin(int8_t pin, uint8_t value) { signals.push_back({pin, value}); }
static void writePwm(int8_t pin, uint16_t value) { signals.push_back({pin, value}); }

static void test_biased_sensor_drives_opposite_correction() {
    PIDController pid;
    const MotorCommand command = pid.update(static_cast<uint16_t>(1u << 0),
                                            FollowMode::CENTER, 0, 50);
    assert(!command.lineLost);
    assert(command.rightSpeed < command.leftSpeed);

    MotorController motors(writePin, writePwm);
    motors.drive(command.rightSpeed, command.leftSpeed);
    assert(signals.size() == 4);
    assert(signals[1].value == static_cast<uint16_t>(command.rightSpeed < 0
        ? -command.rightSpeed : command.rightSpeed));
    assert(signals[3].value == static_cast<uint16_t>(command.leftSpeed < 0
        ? -command.leftSpeed : command.leftSpeed));
}

int main() {
    test_biased_sensor_drives_opposite_correction();
}
