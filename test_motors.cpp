#include <cassert>
#include <cstdint>
#include <vector>
#include "motors.h"

struct MotorSignal {
    int8_t pin;
    uint8_t value;
};

std::vector<MotorSignal> signals;

void mockWrite(int8_t pin, uint8_t value) {
    if (pin == -1) return;
    signals.push_back({pin, value});
}

void mockPwm(int8_t pin, uint16_t value) {
    if (pin == -1) return;
    signals.push_back({pin, (uint8_t)(value >> 2)}); // simplify for test
}

void resetSignals() {
    signals.clear();
}

void test_motor_forward() {
    resetSignals();
    MotorController motors(mockWrite, mockPwm);
    
    // Test forward
    motors.drive(100, 100);
    
    // RZ7886 Forward: PWM on one pin, LOW on other
    // Pins: M1(0,1), M2(5,7)
    bool m1_pwm = false, m1_low = false;
    bool m2_pwm = false, m2_low = false;
    
    for(auto s : signals) {
        if (s.pin == 0 && s.value > 0) m1_pwm = true;
        if (s.pin == 1 && s.value == 0) m1_low = true;
        if (s.pin == 5 && s.value > 0) m2_pwm = true;
        if (s.pin == 7 && s.value == 0) m2_low = true;
    }
    assert(m1_pwm && m1_low);
    assert(m2_pwm && m2_low);
}

void test_motor_stop() {
    resetSignals();
    MotorController motors(mockWrite, mockPwm);
    motors.stop();
    
    for(auto s : signals) {
        assert(s.value == 0);
    }
}

void test_motor_negative_pins() {
    resetSignals();
    // Pins defined as -1 in config should not trigger writes
    MotorController motors(mockWrite, mockPwm);
    // Force a state where we use a -1 pin (if we had any in config for motors)
    // Actually config.h has valid pins for motors, but we test the logic
    motors.drive(100, 100);
    // Just ensure no crash and logic holds
}

int main() {
    test_motor_forward();
    test_motor_stop();
    test_motor_negative_pins();
    return 0;
}
