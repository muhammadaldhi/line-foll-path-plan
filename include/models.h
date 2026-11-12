#ifndef MODELS_H
#define MODELS_H

#include <cstdint>
#include "config.h"
#include "menu.h"

enum class Sensor { IGN, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14 };
enum class SensorTriggerMode { NONE, DIRECT, EQ, OR, XOR };
enum class SensorMask : uint16_t { NONE = 0, ALL = 0x3fff };
inline constexpr uint16_t to_underlying(SensorMask m) { return static_cast<uint16_t>(m); }
inline constexpr bool operator==(SensorMask a, uint16_t b) { return static_cast<uint16_t>(a) == b; }
inline constexpr bool operator==(uint16_t a, SensorMask b) { return a == static_cast<uint16_t>(b); }
inline constexpr bool operator==(SensorMask a, SensorMask b) { return static_cast<uint16_t>(a) == static_cast<uint16_t>(b); }
enum class CounterMode { NONE, TIMER, ENCODERR, ENCODERL };
enum class FollowMode { NONE, CENTER, LEFT, RIGHT };

struct StepPlan {
    Action action;
    Sensor sensor;
    SensorTriggerMode sensorTriggerMode;
    SensorMask sensorMaskA;
    SensorMask sensorMaskB;
    int16_t rightSpeed;
    int16_t leftSpeed;
    CounterMode counterMode;
    uint32_t counter;
    int32_t valueA;
    int32_t counterA;
    CounterMode counterAMode;
    int32_t valueB;
    int32_t counterB;
    CounterMode counterBMode;
    uint8_t pidProfile;
    int16_t pidKi;
    FollowMode followMode;
    bool jumpEnabled;
    bool runForever;
    uint8_t jumpTargetMemory;
    uint16_t jumpTargetStep;
};

struct MemoryProgram { StepPlan steps[STEP_COUNT]; uint16_t stopPlan; };

inline void initializeDefaultStep(StepPlan& step) {
    step.action = Action::FINISH;
    step.sensor = Sensor::IGN;
    step.sensorTriggerMode = SensorTriggerMode::NONE;
    step.sensorMaskA = SensorMask::NONE;
    step.sensorMaskB = SensorMask::NONE;
    step.rightSpeed = 0; step.leftSpeed = 0;
    step.counterMode = CounterMode::NONE; step.counter = 0;
    step.valueA = 0; step.counterA = 0; step.counterAMode = CounterMode::NONE;
    step.valueB = 0; step.counterB = 0; step.counterBMode = CounterMode::NONE;
    step.pidProfile = 0; step.pidKi = 0; step.followMode = FollowMode::NONE;
    step.jumpEnabled = false; step.runForever = false; step.jumpTargetMemory = 0; step.jumpTargetStep = 0;
}
inline void initializeDefaultMemory(MemoryProgram& memory) {
    memory.stopPlan = STEP_COUNT;
    for (uint16_t i = 0; i < STEP_COUNT; ++i) initializeDefaultStep(memory.steps[i]);
}
#endif
