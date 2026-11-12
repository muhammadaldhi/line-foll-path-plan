#include "planner.h"
#include "menu.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "buttons.h"
#include "display.h"
#include "encoder.h"
#include "motors.h"
#include "pid.h"
#include "sensors.h"
#include "storage.h"
namespace {
Display display;
PIDController pidController;
EncoderController encoderController;
uint32_t lastPidTime = 0;
constexpr uint32_t PID_INTERVAL_MS = 5;
int buttonInput(Button button) { return static_cast<int>(button); }
uint16_t activeMaskFromValues(const SensorValues& values) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        if (values[i] != 0) mask |= static_cast<uint16_t>(1u << i);
    }
    return mask;
}
}
#endif

RobotPlanner alwi;

RobotPlanner::RobotPlanner() : currentMemory_(0), currentStep_(0) {}

void RobotPlanner::begin() {
#ifdef ARDUINO
    initializeStorage();
    configureMemoryEditor({
        [](uint8_t index, MemoryProgram& memory) { return loadMemory(index, memory); },
        [](uint8_t index, const MemoryProgram& memory) { return saveMemory(index, memory); }
    });
    initializeMotors();
    initializeSensors();
    initializeButtons();
    display.initialize();
    lastPidTime = millis();
#endif
}

void RobotPlanner::loop() {
#ifdef ARDUINO
    const uint32_t now = millis();
    static MenuState menuState{};
    static PlannerRuntime planner(activeMemory());
    static bool started = false;

    processMenuInput(menuState, buttonInput(readButtons()));
    autosaveTick(now);

    const SensorValues sensors = readSensors();
    uint16_t sensorValues[SENSOR_COUNT] = {};
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) sensorValues[i] = sensors[i];
    const MainScreenValues screenValues{
        menuState.indexMemory, menuState.indexCP, menuState.indexPlan, menuState.indexStopPlan
    };
    display.renderMainScreen(menuState.position, sensorValues, screenValues);

    if (menuState.position == MenuPosition::RUN_CHECKPOINT) {
        if (!started) {
            planner.reset();
            pidController.reset();
            started = true;
        }
        RuntimeInput input;
        input.sensorMask = activeMaskFromValues(sensors);
        input.rightEncoder = encoderController.rightCount();
        input.leftEncoder = encoderController.leftCount();
        const RuntimeResult result = planner.step(activeMemory(), input);
        if (result.status == RuntimeStatus::RUNNING && now - lastPidTime >= PID_INTERVAL_MS) {
            const RuntimeCommand& command = result.command;
            const uint8_t baseSpeed = static_cast<uint8_t>((command.rightSpeed + command.leftSpeed) / 2);
            const MotorCommand motor = pidController.update(
                input.sensorMask, command.followMode, command.pidProfile, baseSpeed, now, PID_INTERVAL_MS);
            if (motor.lineLost) hardwareMotors().stop();
            else hardwareMotors().drive(motor.rightSpeed, motor.leftSpeed);
            lastPidTime = now;
        } else if (result.status != RuntimeStatus::RUNNING) {
            hardwareMotors().stop();
            started = false;
        }
    } else {
        hardwareMotors().stop();
        started = false;
    }
    delay(2);
#endif
}

RobotPlanner& RobotPlanner::memory(uint8_t memoryIndex) {
    if (memoryIndex < MEMORY_COUNT) {
        currentMemory_ = memoryIndex;
        selectMemory(memoryIndex);
    }
    return *this;
}

RobotPlanner& RobotPlanner::step(uint16_t stepIndex) {
    if (stepIndex < STEP_COUNT) {
        currentStep_ = stepIndex;
    }
    return *this;
}

RobotPlanner& RobotPlanner::pid(uint8_t profileIndex, int16_t kp, int16_t ki, int16_t kd) {
    if (profileIndex < CALIBRATION_PROFILE_COUNT) {
#ifdef ARDUINO
        pidController.setProfile(profileIndex, {kp, ki, kd});
#else
        (void)kp;
        (void)kd;
#endif
        if (currentStep_ < STEP_COUNT) {
            MemoryProgram& mem = activeMemory();
            mem.steps[currentStep_].pidProfile = profileIndex;
            mem.steps[currentStep_].pidKi = ki;
            markMemoryDirty(0);
        }
    }
    return *this;
}

RobotPlanner& RobotPlanner::forward(SensorLogic logic, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs) {
    SensorPreset preset = getSensorPreset(logic);
    return forward(preset.mode, to_underlying(preset.maskA), to_underlying(preset.maskB), leftSpeed, rightSpeed, delayMs);
}

RobotPlanner& RobotPlanner::forward(SensorTriggerMode mode, uint16_t maskA, uint16_t maskB, int16_t leftSpeed, int16_t rightSpeed, uint32_t delayMs) {
    if (currentStep_ < STEP_COUNT) {
        MemoryProgram& mem = activeMemory();
        StepPlan& plan = mem.steps[currentStep_];
        plan.action = Action::MOVE_FWD;
        plan.sensorTriggerMode = mode;
        plan.sensorMaskA = static_cast<SensorMask>(maskA);
        plan.sensorMaskB = static_cast<SensorMask>(maskB);
        plan.leftSpeed = leftSpeed;
        plan.rightSpeed = rightSpeed;
        plan.counterMode = CounterMode::TIMER;
        plan.counter = delayMs;
        markMemoryDirty(0);
    }
    return *this;
}

RobotPlanner& RobotPlanner::afterForward(int16_t speedAfter, uint32_t delay2Ms) {
    if (currentStep_ < STEP_COUNT) {
        MemoryProgram& mem = activeMemory();
        StepPlan& plan = mem.steps[currentStep_];
        plan.valueA = speedAfter;
        plan.counterA = delay2Ms;
        plan.counterAMode = CounterMode::TIMER;
        markMemoryDirty(0);
    }
    return *this;
}

RobotPlanner& RobotPlanner::usePid(uint8_t profileIndex) {
    if (currentStep_ < STEP_COUNT) {
        MemoryProgram& mem = activeMemory();
        mem.steps[currentStep_].pidProfile = profileIndex;
        markMemoryDirty(0);
    }
    return *this;
}

RobotPlanner& RobotPlanner::lineMode(LineColor color) {
    (void)color;
    return *this;
}

RobotPlanner& RobotPlanner::stopAfterAction() {
    MemoryProgram& mem = activeMemory();
    mem.stopPlan = currentStep_ + 1;
    markMemoryDirty(0);
    return *this;
}

RobotPlanner& RobotPlanner::save() {
    flushPendingSave();
    return *this;
}

bool shouldStopBeforeStep(const MemoryProgram& memory, uint16_t currentStep) {
    return memory.stopPlan < STEP_COUNT && currentStep >= memory.stopPlan;
}

bool shouldStopAfterStep(const StepPlan& step) {
    return step.action == Action::FINISH;
}

uint16_t nextStepIndex(uint16_t currentStep) {
    return static_cast<uint16_t>(currentStep + 1);
}

SensorPreset getSensorPreset(SensorLogic logic) {
    switch (logic) {
    case SensorLogic::NONE:
        return {SensorTriggerMode::NONE, SensorMask::NONE, SensorMask::NONE};
    case SensorLogic::DIRECT:
        return {SensorTriggerMode::DIRECT, SensorMask::NONE, SensorMask::NONE};
    case SensorLogic::EQ_NONE:
        return {SensorTriggerMode::EQ, static_cast<SensorMask>(0x0000), SensorMask::NONE};
    case SensorLogic::EQ_ALL:
        return {SensorTriggerMode::EQ, static_cast<SensorMask>(0x3fff), SensorMask::NONE};
    case SensorLogic::OR_LEFT_1:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x0001), SensorMask::NONE};
    case SensorLogic::OR_LEFT_2:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x0003), SensorMask::NONE};
    case SensorLogic::OR_LEFT_3:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x0007), SensorMask::NONE};
    case SensorLogic::OR_LEFT_4:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x000f), SensorMask::NONE};
    case SensorLogic::OR_RIGHT_1:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x2000), SensorMask::NONE};
    case SensorLogic::OR_RIGHT_2:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x3000), SensorMask::NONE};
    case SensorLogic::OR_RIGHT_3:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x3800), SensorMask::NONE};
    case SensorLogic::OR_RIGHT_4:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x3c00), SensorMask::NONE};
    case SensorLogic::OR_ALL:
        return {SensorTriggerMode::OR, static_cast<SensorMask>(0x3fff), SensorMask::NONE};
    case SensorLogic::XOR_CENTER_LEFT_1:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x0001)};
    case SensorLogic::XOR_CENTER_LEFT_2:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x0003)};
    case SensorLogic::XOR_CENTER_LEFT_3:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x0007)};
    case SensorLogic::XOR_CENTER_LEFT_4:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x000f)};
    case SensorLogic::XOR_CENTER_RIGHT_1:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x2000)};
    case SensorLogic::XOR_CENTER_RIGHT_2:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x3000)};
    case SensorLogic::XOR_CENTER_RIGHT_3:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x3800)};
    case SensorLogic::XOR_CENTER_RIGHT_4:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x01e0), static_cast<SensorMask>(0x3c00)};
    case SensorLogic::XOR_LEFT_1_RIGHT_1:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x2000), static_cast<SensorMask>(0x0001)};
    case SensorLogic::XOR_LEFT_2_RIGHT_2:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x3000), static_cast<SensorMask>(0x0003)};
    case SensorLogic::XOR_LEFT_3_RIGHT_3:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x3800), static_cast<SensorMask>(0x0007)};
    case SensorLogic::XOR_LEFT_4_RIGHT_4:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x3c00), static_cast<SensorMask>(0x000f)};
    case SensorLogic::XOR_LEFT_5_RIGHT_5:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x3e00), static_cast<SensorMask>(0x001f)};
    case SensorLogic::XOR_LEFT_6_RIGHT_6:
        return {SensorTriggerMode::XOR, static_cast<SensorMask>(0x3f00), static_cast<SensorMask>(0x003f)};
    }
    return {SensorTriggerMode::NONE, SensorMask::NONE, SensorMask::NONE};
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
