#include "planner.h"

bool shouldStopBeforeStep(const MemoryProgram& memory, uint16_t currentStep) {
    return memory.stopPlan < STEP_COUNT && currentStep >= memory.stopPlan;
}

bool shouldStopAfterStep(const StepPlan& step) {
    return step.action == Action::FINISH;
}

uint16_t nextStepIndex(uint16_t currentStep) {
    return static_cast<uint16_t>(currentStep + 1);
}

namespace {
bool validMask(uint16_t mask) { return (mask & ~to_underlying(SensorMask::ALL)) == 0; }
bool triggered(const StepPlan& step, uint16_t sensors) {
    const uint16_t a = to_underlying(step.sensorMaskA);
    const uint16_t b = to_underlying(step.sensorMaskB);
    switch (step.sensorTriggerMode) {
    case SensorTriggerMode::NONE: return true;
    case SensorTriggerMode::DIRECT: return true;
    case SensorTriggerMode::EQ: return sensors == a;
    case SensorTriggerMode::OR: return (sensors & (a | b)) != 0;
    case SensorTriggerMode::XOR: return (sensors & a) != 0 && (sensors & b) != 0;
    }
    return false;
}
}

PlannerRuntime::PlannerRuntime(const MemoryProgram& memory) : step_(0), memory_(&memory) {}
void PlannerRuntime::reset(uint16_t step) { step_ = step; }
uint16_t PlannerRuntime::currentStep() const { return step_; }
RuntimeResult PlannerRuntime::step(const MemoryProgram& memory, const RuntimeInput& input) {
    RuntimeResult result;
    memory_ = &memory;
    if (input.emergencyStop) { result.status = RuntimeStatus::EMERGENCY_STOP; return result; }
    if (step_ >= STEP_COUNT || shouldStopBeforeStep(memory, step_)) {
        result.status = RuntimeStatus::FINISHED; return result;
    }
    const StepPlan& plan = memory.steps[step_];
    if (!validMask(to_underlying(plan.sensorMaskA)) || !validMask(to_underlying(plan.sensorMaskB))) {
        result.status = RuntimeStatus::FAULT; return result;
    }
    if (!triggered(plan, input.sensorMask)) return result;
    result.command.action = plan.action;
    result.command.rightSpeed = plan.rightSpeed;
    result.command.leftSpeed = plan.leftSpeed;
    result.command.counterMode = plan.counterMode;
    result.command.counter = plan.counter;
    result.command.hasPhaseA = plan.counterA > 0 && plan.counterAMode != CounterMode::NONE;
    result.command.hasPhaseB = plan.counterB > 0 && plan.counterBMode != CounterMode::NONE;
    result.command.valueA = plan.valueA; result.command.counterA = plan.counterA;
    result.command.valueB = plan.valueB; result.command.counterB = plan.counterB;
    result.command.counterAMode = plan.counterAMode; result.command.counterBMode = plan.counterBMode;
    result.command.followMode = plan.followMode; result.command.pidProfile = plan.pidProfile;
    result.command.pidKi = plan.pidKi;
    result.command.lineModeChange = plan.action == Action::NONE || plan.action == Action::MOVE_FWD || plan.action == Action::MOVE_BWD;
    if (plan.jumpEnabled) {
        if (plan.jumpTargetMemory >= MEMORY_COUNT || plan.jumpTargetStep >= STEP_COUNT) {
            result.status = RuntimeStatus::FAULT; return result;
        }
        result.targetMemory = plan.jumpTargetMemory; result.targetStep = plan.jumpTargetStep;
        step_ = plan.jumpTargetStep;
    } else {
        step_ = nextStepIndex(step_);
    }
    if (plan.action == Action::FINISH) result.status = RuntimeStatus::FINISHED;
    return result;
}
