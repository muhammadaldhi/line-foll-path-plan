#ifndef PLANNER_H
#define PLANNER_H

#include <cstdint>
#include "models.h"

bool shouldStopBeforeStep(const MemoryProgram& memory, uint16_t currentStep);
bool shouldStopAfterStep(const StepPlan& step);
uint16_t nextStepIndex(uint16_t currentStep);

enum class RuntimeStatus { RUNNING, FINISHED, EMERGENCY_STOP, FAULT };

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

class PlannerRuntime {
public:
    explicit PlannerRuntime(const MemoryProgram& memory);
    void reset(uint16_t step = 0);
    RuntimeResult step(const MemoryProgram& memory, const RuntimeInput& input);
    uint16_t currentStep() const;
private:
    uint16_t step_;
    const MemoryProgram* memory_;
};

#endif
