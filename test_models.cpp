#include <cassert>
#include <cstdint>
#include <type_traits>
#include "config.h"
#include "models.h"

static void test_capacities() {
    static_assert(MEMORY_COUNT == 16);
    static_assert(STEP_COUNT == 150);
    static_assert(CHECKPOINT_COUNT == 30);
    static_assert(CALIBRATION_PROFILE_COUNT == 5);
}

static void test_legacy_parameter_model() {
    static_assert(static_cast<uint8_t>(SensorTriggerMode::XOR) == 4);
    static_assert(static_cast<uint16_t>(SensorMask::ALL) == 0x3fff);
    static_assert(static_cast<uint8_t>(CounterMode::TIMER) == 1);
    static_assert(static_cast<uint8_t>(CounterMode::ENCODERR) == 2);
    static_assert(static_cast<uint8_t>(CounterMode::ENCODERL) == 3);
    static_assert(FollowMode::CENTER != FollowMode::LEFT);
    static_assert(FollowMode::LEFT != FollowMode::RIGHT);

    StepPlan step{};
    initializeDefaultStep(step);
    assert(step.action == Action::FINISH);
    assert(step.sensor == Sensor::IGN);
    assert(step.sensorTriggerMode == SensorTriggerMode::NONE);
    assert(step.sensorMaskA == 0);
    assert(step.sensorMaskB == 0);
    assert(step.counterMode == CounterMode::NONE);
    assert(step.counter == 0);
    assert(step.counterAMode == CounterMode::NONE);
    assert(step.counterBMode == CounterMode::NONE);
    assert(step.followMode == FollowMode::NONE);
    assert(step.jumpEnabled == false);
}

int main() {
    test_capacities();
    test_legacy_parameter_model();
    return 0;
}
