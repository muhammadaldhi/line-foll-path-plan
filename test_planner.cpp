#include <cassert>
#include <type_traits>
#include "planner.h"

int main() {
    static_assert(Action::FINISH != Action::MOVE_FWD);
    static_assert(Action::BLOW != Action::PICK);
    static_assert(CounterMode::ENCODERR != CounterMode::ENCODERL);

    StepPlan step{};
    initializeDefaultStep(step);
    step.sensorTriggerMode = SensorTriggerMode::XOR;
    step.sensorMaskA = static_cast<SensorMask>(1U << 13);
    step.sensorMaskB = static_cast<SensorMask>(1U << 0);
    step.rightSpeed = 100;
    step.leftSpeed = -100;
    step.counterMode = CounterMode::TIMER;
    step.counter = 1234;
    step.valueA = 42;
    step.counterA = 43;
    step.counterAMode = CounterMode::ENCODERR;
    step.valueB = 44;
    step.counterB = 45;
    step.counterBMode = CounterMode::ENCODERL;
    step.pidProfile = 3;
    step.followMode = FollowMode::RIGHT;
    step.jumpEnabled = true;
    step.jumpTargetMemory = 15;
    step.jumpTargetStep = 149;
    assert(step.sensorMaskA == 0x2000);
    assert(step.sensorMaskB == 1);
    assert(step.rightSpeed == 100 && step.leftSpeed == -100);
    assert(step.counter == 1234 && step.counterA == 43 && step.counterB == 45);
    assert(step.jumpTargetMemory == 15 && step.jumpTargetStep == 149);

    MemoryProgram memory{};
    memory.stopPlan = 0;
    assert(shouldStopBeforeStep(memory, 0));
    memory.stopPlan = 20;
    assert(!shouldStopBeforeStep(memory, 0));
    assert(!shouldStopBeforeStep(memory, 19));
    assert(shouldStopBeforeStep(memory, 20));
    memory.stopPlan = 149;
    assert(!shouldStopBeforeStep(memory, 148));
    assert(shouldStopBeforeStep(memory, 149));
    memory.stopPlan = 150;
    assert(!shouldStopBeforeStep(memory, 0));
    assert(!shouldStopBeforeStep(memory, 149));
    step.action = Action::MOVE_FWD;
    assert(!shouldStopAfterStep(step));
    step.action = Action::FINISH;
    assert(shouldStopAfterStep(step));
    assert(nextStepIndex(0) == 1);
    assert(nextStepIndex(149) == 150);

    assert(getSensorPreset(SensorLogic::NONE).mode == SensorTriggerMode::NONE);
    assert(getSensorPreset(SensorLogic::DIRECT).mode == SensorTriggerMode::DIRECT);
    assert(to_underlying(getSensorPreset(SensorLogic::EQ_NONE).maskA) == 0x0000);
    assert(to_underlying(getSensorPreset(SensorLogic::EQ_ALL).maskA) == 0x3fff);
    assert(to_underlying(getSensorPreset(SensorLogic::OR_LEFT_4).maskA) == 0x000f);
    assert(to_underlying(getSensorPreset(SensorLogic::OR_RIGHT_4).maskA) == 0x3c00);
    assert(to_underlying(getSensorPreset(SensorLogic::XOR_CENTER_LEFT_4).maskA) == 0x01e0);
    assert(to_underlying(getSensorPreset(SensorLogic::XOR_CENTER_LEFT_4).maskB) == 0x000f);
    assert(to_underlying(getSensorPreset(SensorLogic::XOR_LEFT_6_RIGHT_6).maskA) == 0x3f00);
    assert(to_underlying(getSensorPreset(SensorLogic::XOR_LEFT_6_RIGHT_6).maskB) == 0x003f);

    alwi.memory(0)
        .step(1)
        .forward(SensorLogic::XOR_CENTER_LEFT_4, 180, 180, 50)
        .afterForward(225, 20)
        .usePid(1)
        .lineMode(LineColor::BLACK)
        .stopAfterAction()
        .save();

    const StepPlan& configured = activeMemory().steps[1];
    assert(configured.action == Action::MOVE_FWD);
    assert(configured.sensorTriggerMode == SensorTriggerMode::XOR);
    assert(to_underlying(configured.sensorMaskA) == 0x01e0);
    assert(to_underlying(configured.sensorMaskB) == 0x000f);
    assert(configured.leftSpeed == 180 && configured.rightSpeed == 180);
    assert(configured.counterMode == CounterMode::TIMER && configured.counter == 50);
    assert(configured.valueA == 225 && configured.counterA == 20);
    assert(configured.pidProfile == 1);
    assert(activeMemory().stopPlan == 2);

    initializeDefaultMemory(memory);
    PlannerRuntime runtime(memory);
    RuntimeInput input{};
    StepPlan direct{};
    initializeDefaultStep(direct);
    direct.action = Action::MOVE_FWD;
    direct.sensorTriggerMode = SensorTriggerMode::DIRECT;
    direct.sensorMaskA = SensorMask::NONE;
    memory.steps[0] = direct;
    runtime.reset(0);
    RuntimeResult directResult = runtime.step(memory, input);
    assert(directResult.command.action == Action::MOVE_FWD);

    StepPlan trigger{};
    initializeDefaultStep(trigger);
    trigger.action = Action::PICK;
    trigger.sensorTriggerMode = SensorTriggerMode::EQ;
    trigger.sensorMaskA = static_cast<SensorMask>(0x0003);
    trigger.counterMode = CounterMode::TIMER;
    trigger.counter = 20;
    trigger.valueA = 10;
    trigger.counterA = 4;
    trigger.counterAMode = CounterMode::ENCODERR;
    trigger.valueB = 20;
    trigger.counterB = 5;
    trigger.counterBMode = CounterMode::ENCODERL;
    trigger.followMode = FollowMode::LEFT;
    trigger.pidProfile = 2;
    trigger.pidKi = 17;
    trigger.pidKi = 17;
    memory.steps[0] = trigger;
    memory.stopPlan = STEP_COUNT;
    runtime.reset(0);
    input.sensorMask = 0x0003;
    input.rightEncoder = 4;
    input.leftEncoder = 5;
    RuntimeResult result = runtime.step(memory, input);
    assert(result.status == RuntimeStatus::RUNNING);
    assert(result.command.action == Action::PICK);
    assert(result.command.counterMode == CounterMode::TIMER);
    assert(result.command.hasPhaseA && result.command.hasPhaseB);
    assert(result.command.followMode == FollowMode::LEFT);
    assert(result.command.pidProfile == 2);
    assert(result.command.pidKi == 17);

    trigger.sensorTriggerMode = SensorTriggerMode::OR;
    trigger.sensorMaskA = static_cast<SensorMask>(0x0004);
    trigger.sensorMaskB = static_cast<SensorMask>(0x0008);
    memory.steps[1] = trigger;
    runtime.reset(1);
    input.sensorMask = 0x0008;
    result = runtime.step(memory, input);
    assert(result.command.action == Action::PICK);

    memory.steps[2] = trigger;
    memory.steps[2].jumpEnabled = true;
    memory.steps[2].jumpTargetMemory = MEMORY_COUNT;
    runtime.reset(2);
    result = runtime.step(memory, input);
    assert(result.status == RuntimeStatus::FAULT);

    memory.steps[3] = trigger;
    memory.steps[3].action = Action::FINISH;
    runtime.reset(3);
    result = runtime.step(memory, input);
    assert(result.status == RuntimeStatus::FINISHED);

    input.emergencyStop = true;
    result = runtime.step(memory, input);
    assert(result.status == RuntimeStatus::EMERGENCY_STOP);
    return 0;
}
