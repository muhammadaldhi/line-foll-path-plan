#ifndef PLANNER_H
#define PLANNER_H

#include <cstdint>
#include "models.h"

bool shouldStopBeforeStep(const MemoryProgram& memory, uint16_t currentStep);
bool shouldStopAfterStep(const StepPlan& step);
uint16_t nextStepIndex(uint16_t currentStep);

enum class RuntimeStatus { RUNNING, FINISHED, EMERGENCY_STOP, FAULT };

enum class SensorLogic {
    NONE, DIRECT, EQ_NONE, EQ_ALL,
    OR_LEFT_1, OR_LEFT_2, OR_LEFT_3, OR_LEFT_4,
    OR_RIGHT_1, OR_RIGHT_2, OR_RIGHT_3, OR_RIGHT_4, OR_ALL,
    XOR_CENTER_LEFT_1, XOR_CENTER_LEFT_2, XOR_CENTER_LEFT_3, XOR_CENTER_LEFT_4,
    XOR_CENTER_RIGHT_1, XOR_CENTER_RIGHT_2, XOR_CENTER_RIGHT_3, XOR_CENTER_RIGHT_4,
    XOR_LEFT_1_RIGHT_1, XOR_LEFT_2_RIGHT_2, XOR_LEFT_3_RIGHT_3,
    XOR_LEFT_4_RIGHT_4, XOR_LEFT_5_RIGHT_5, XOR_LEFT_6_RIGHT_6
};

struct SensorPreset {
    SensorTriggerMode mode;
    SensorMask maskA;
    SensorMask maskB;
};

SensorPreset getSensorPreset(SensorLogic logic);

struct RuntimeInput {
    uint16_t sensorMask = 0;
    int32_t rightEncoder = 0;
    int32_t leftEncoder = 0;
    bool emergencyStop = false;
};

struct RuntimeCommand {
    Action action = Action::NONE;
    int16_t rightSpeed = 0;
    int16_t leftSpeed = 0;
    CounterMode counterMode = CounterMode::NONE;
    uint32_t counter = 0;
    bool hasPhaseA = false;
    bool hasPhaseB = false;
    int32_t valueA = 0;
    int32_t valueB = 0;
    int32_t counterA = 0;
    int32_t counterB = 0;
    CounterMode counterAMode = CounterMode::NONE;
    CounterMode counterBMode = CounterMode::NONE;
    FollowMode followMode = FollowMode::NONE;
    uint8_t pidProfile = 0;
    int16_t pidKi = 0;
    bool lineModeChange = false;
};

struct RuntimeResult {
    RuntimeStatus status = RuntimeStatus::RUNNING;
    RuntimeCommand command{};
    uint8_t targetMemory = 0;
    uint16_t targetStep = 0;
};

enum class LineColor { BLACK = 0, WHITE = 1 };

class RobotPlanner {
public:
    RobotPlanner();

    void begin();
    void loop();

    RobotPlanner& pid(uint8_t profileIndex, int16_t kp, int16_t ki, int16_t kd);
    RobotPlanner& memory(uint8_t memoryIndex);
    RobotPlanner& step(uint16_t stepIndex);

    RobotPlanner& forward(SensorLogic logic, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);
    RobotPlanner& forward(SensorTriggerMode mode, uint16_t maskA, uint16_t maskB, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);

    RobotPlanner& backward(SensorLogic logic, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);
    RobotPlanner& backward(SensorTriggerMode mode, uint16_t maskA, uint16_t maskB, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);

    RobotPlanner& turnLeft(SensorLogic logic, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);
    RobotPlanner& turnLeft(SensorTriggerMode mode, uint16_t maskA, uint16_t maskB, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);

    RobotPlanner& turnRight(SensorLogic logic, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);
    RobotPlanner& turnRight(SensorTriggerMode mode, uint16_t maskA, uint16_t maskB, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs);

    RobotPlanner& finish();
    RobotPlanner& runForever();

    RobotPlanner& afterForward(int16_t speedAfter, uint32_t delay2Ms);
    RobotPlanner& usePid(uint8_t profileIndex);
    RobotPlanner& lineMode(LineColor color);

    RobotPlanner& stopAfterAction();
    RobotPlanner& save();

private:
    uint8_t currentMemory_ = 0;
    uint16_t currentStep_ = 0;
};

extern RobotPlanner alwi;

class PlannerRuntime {
public:
    explicit PlannerRuntime(const MemoryProgram& memory);
    void reset(uint16_t step = 0);
    RuntimeResult step(const MemoryProgram& memory, const RuntimeInput& input, uint32_t nowMs = 0);
    uint16_t currentStep() const;
private:
    uint16_t step_;
    const MemoryProgram* memory_;
    bool actionActive_ = false;
    uint32_t actionStartedAt_ = 0;
    int32_t rightEncoderStart_ = 0;
    int32_t leftEncoderStart_ = 0;
};

#endif
