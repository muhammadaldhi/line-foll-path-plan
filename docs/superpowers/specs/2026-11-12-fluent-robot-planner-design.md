# Design Spec: Fluent RobotPlanner API & Simplified main.cpp

## Intent & Why
Provide a fluent C++ API (`RobotPlanner alwi`) allowing line follower configuration directly in `setup()` using chainable method calls, while keeping the UI, sensors, motors, PID, and LittleFS persistence fully active and synchronized.

## Goals
- Support fluent chaining for PID profile definition, speed aliases, memory/step selection, step plan parameters, and stop action.
- Automatically synchronize code-configured step plans with `activeMemory()` and LittleFS persistence (`autosaveTick` / `flushPendingSave`).
- Keep `setup()` and `loop()` concise while delegating all hardware initialization and runtime execution to `alwi.begin()` and `alwi.loop()`.
- Maintain host-test compatibility for non-Arduino build targets.

## Non-Goals
- Changing the underlying LittleFS binary format (`MemoryProgram` / `StepPlan`).
- Modifying UI bitmap coordinates or menu layout structure.

## Architecture & API
### Class `RobotPlanner`
```cpp
class RobotPlanner {
public:
    RobotPlanner();

    void begin();
    void loop();

    RobotPlanner& pid(uint8_t profileIndex, float kp, float ki, float kd);
    RobotPlanner& speed(const char* name, int16_t speedValue);

    RobotPlanner& memory(uint8_t memoryIndex);
    RobotPlanner& step(uint16_t stepIndex);

    RobotPlanner& forward(Logic logic, int16_t leftSpeed, int16_t rightSpeed, uint16_t delayMs);
    RobotPlanner& afterForward(int16_t speedAfter, uint16_t delay2Ms);
    RobotPlanner& usePid(uint8_t profileIndex);
    RobotPlanner& lineMode(LineColor color);

    RobotPlanner& stopAfterAction();
    RobotPlanner& save();
};

extern RobotPlanner alwi;
```

### Main Entry Point (`src/main.cpp`)
```cpp
#include <Arduino.h>
#include "planner.h"

void setup() {
    Serial.begin(115200);

    alwi.begin();

    alwi.pid(1, 0.8f, 10.0f, 1.0f)
        .pid(2, 2.0f, 10.0f, 1.0f)
        .pid(3, 6.0f, 11.0f, 3.0f)
        .pid(4, 2.0f, 0.0f, 4.0f)
        .memory(0)
        .step(1)
        .forward(Logic::XOR1, 180, 180, 50)
        .afterForward(225, 20)
        .usePid(1)
        .lineMode(LineColor::BLACK)
        .stopAfterAction()
        .save();
}

void loop() {
    alwi.loop();
}
```

## Storage & UI Sync
When `alwi.save()` or step setters are called:
1. Target memory is updated in `activeMemory()`.
2. `markMemoryDirty(millis())` is called so LittleFS auto-save persists the changes.
3. UI rendering reads directly from `activeMemory()`, ensuring UI displays the newly set values.
