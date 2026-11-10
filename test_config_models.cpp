#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include "config.h"
#include "models.h"

static void testConfigConstants() {
    static_assert(SENSOR_COUNT == 14);
    static_assert(MEMORY_COUNT == 16);
    static_assert(CHECKPOINT_COUNT == 30);
    static_assert(STEP_COUNT == 150);
    static_assert(CALIBRATION_PROFILE_COUNT == 5);
}

static void testPinConstants() {
    static_assert(sizeof(PWMR2) > 0);
    static_assert(sizeof(PWMR1) > 0);
    static_assert(sizeof(SEL1) > 0);
    static_assert(sizeof(SEL0) > 0);
    static_assert(sizeof(OUT_MUX) > 0);
    static_assert(sizeof(PWML1) > 0);
    static_assert(sizeof(SEL3) > 0);
    static_assert(sizeof(PWML2) > 0);
    static_assert(sizeof(SEL2) > 0);
    static_assert(sizeof(TFT_SCK) > 0);
    static_assert(sizeof(TFT_MOSI) > 0);
    static_assert(sizeof(TFT_RES) > 0);
    static_assert(sizeof(TFT_DC) > 0);
    static_assert(SERVO_L_PIN == -1);
    static_assert(SERVO_R_PIN == -1);
    static_assert(ENC_L_A_PIN == -1);
    static_assert(ENC_L_B_PIN == -1);
    static_assert(ENC_R_A_PIN == -1);
    static_assert(ENC_R_B_PIN == -1);
    static_assert(BUZZER_PIN == -1);
}

static void testModelTypes() {
    static_assert(std::is_same<decltype(MemoryProgram::steps), StepPlan[STEP_COUNT]>::value);
    static_assert(std::is_same<decltype(MemoryProgram::stopPlan), uint16_t>::value);
    static_assert(std::is_same<decltype(StepPlan::action), Action>::value);
    static_assert(std::is_same<decltype(StepPlan::sensor), Sensor>::value);
    static_assert(std::is_same<decltype(StepPlan::rightSpeed), int16_t>::value);
    static_assert(std::is_same<decltype(StepPlan::leftSpeed), int16_t>::value);
    static_assert(std::is_same<decltype(StepPlan::counter), uint32_t>::value);
    static_assert(std::is_same<decltype(StepPlan::jumpTargetStep), uint16_t>::value);
}

static void testDefaultModels() {
    StepPlan step{};
    initializeDefaultStep(step);
    assert(step.action == Action::FINISH);
    assert(step.sensor == Sensor::IGN);
    assert(step.rightSpeed == 0);
    assert(step.leftSpeed == 0);
    assert(step.counterMode == CounterMode::NONE);
    assert(step.counter == 0);
    assert(step.valueA == 0);
    assert(step.counterA == 0);
    assert(step.counterAMode == CounterMode::NONE);
    assert(step.valueB == 0);
    assert(step.counterB == 0);
    assert(step.counterBMode == CounterMode::NONE);
    assert(step.pidProfile == 0);
    assert(step.followMode == FollowMode::NONE);
    assert(!step.jumpEnabled);
    assert(step.jumpTargetMemory == 0);
    assert(step.jumpTargetStep == 0);

    MemoryProgram memory{};
    initializeDefaultMemory(memory);
    assert(memory.stopPlan == STEP_COUNT);
    for (uint16_t i = 0; i < STEP_COUNT; ++i) {
        assert(memory.steps[i].action == Action::FINISH);
        assert(memory.steps[i].sensor == Sensor::IGN);
        assert(memory.steps[i].counterMode == CounterMode::NONE);
        assert(memory.steps[i].followMode == FollowMode::NONE);
        assert(!memory.steps[i].jumpEnabled);
    }
}

int main() {
    testConfigConstants();
    testPinConstants();
    testModelTypes();
    testDefaultModels();
    std::cout << "All configuration and model tests passed!\n";
    return 0;
}
