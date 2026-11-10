#include <cassert>
#include <cstdint>
#include <vector>

#include "servo.h"

struct ServoSignal {
    int8_t pin;
    uint8_t angle;
};

void test_angles_are_clamped_and_recorded() {
    std::vector<ServoSignal> signals;
    ServoController servos([&signals](int8_t pin, uint8_t angle) {
        signals.push_back({pin, angle});
    }, 10, 11);

    servos.setLeft(-1);
    servos.setRight(181);

    assert(servos.leftAngle() == 0);
    assert(servos.rightAngle() == 180);
    assert(signals.size() == 2);
    assert(signals[0].pin == 10);
    assert(signals[0].angle == 0);
    assert(signals[1].pin == 11);
    assert(signals[1].angle == 180);
}

void test_stop_centers_both_servos() {
    std::vector<ServoSignal> signals;
    ServoController servos([&signals](int8_t pin, uint8_t angle) {
        signals.push_back({pin, angle});
    }, 10, 11);

    servos.setLeft(20);
    servos.setRight(160);
    signals.clear();
    servos.stop();

    assert(servos.leftAngle() == 90);
    assert(servos.rightAngle() == 90);
    assert(signals.size() == 2);
    assert(signals[0].pin == 10);
    assert(signals[0].angle == 90);
    assert(signals[1].pin == 11);
    assert(signals[1].angle == 90);
}

void test_unassigned_pins_keep_state_without_callback() {
    std::vector<ServoSignal> signals;
    ServoController servos([&signals](int8_t pin, uint8_t angle) {
        signals.push_back({pin, angle});
    }, -1, -1);

    servos.setLeft(45);
    servos.setRight(135);
    servos.stop();

    assert(servos.leftAngle() == 90);
    assert(servos.rightAngle() == 90);
    assert(signals.empty());
}

int main() {
    test_angles_are_clamped_and_recorded();
    test_stop_centers_both_servos();
    test_unassigned_pins_keep_state_without_callback();
    return 0;
}
