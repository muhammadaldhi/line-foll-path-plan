#include "config.h"
#include "models.h"
#include "storage.h"
#include "menu.h"
#include "display.h"
#include "sensors.h"
#include "buttons.h"
#include "motors.h"
#include "pid.h"
#include "encoder.h"
#include "planner.h"

#ifdef ARDUINO
#include <Arduino.h>

namespace {
Display display;
PIDController pidController;
EncoderController encoderController;
uint32_t lastPidTime = 0;
constexpr uint32_t PID_INTERVAL_MS = 5;

int buttonInput(Button button) {
    return static_cast<int>(button);
}

uint16_t activeMaskFromValues(const SensorValues& values) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        if (values[i] != 0) mask |= static_cast<uint16_t>(1u << i);
    }
    return mask;
}
} // namespace

void setup() {
    Serial.begin(115200);
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
}

void loop() {
    const uint32_t now = millis();
    static MenuState menuState{};

    processMenuInput(menuState, buttonInput(readButtons()));
    autosaveTick(now);

    const SensorValues sensors = readSensors();
    uint16_t sensorValues[SENSOR_COUNT] = {};
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) sensorValues[i] = sensors[i];

    const MainScreenValues screenValues{
        menuState.indexMemory,
        menuState.indexCP,
        menuState.indexPlan,
        menuState.indexStopPlan
    };
    display.renderMainScreen(menuState.position, sensorValues, screenValues);

    if (menuState.position == MenuPosition::RUN_CHECKPOINT) {
        static PlannerRuntime planner(activeMemory());
        static bool started = false;
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
            const uint8_t baseSpeed = static_cast<uint8_t>(
                (command.rightSpeed + command.leftSpeed) / 2);
            const MotorCommand motor = pidController.update(
                input.sensorMask, command.followMode, command.pidProfile,
                baseSpeed, now, PID_INTERVAL_MS);
            if (motor.lineLost) hardwareMotors().stop();
            else hardwareMotors().drive(motor.rightSpeed, motor.leftSpeed);
            lastPidTime = now;
        } else if (result.status != RuntimeStatus::RUNNING) {
            hardwareMotors().stop();
            started = false;
        }
    } else {
        hardwareMotors().stop();
    }
    delay(2);
}

#else
// Host builds compile this translation unit without Arduino or hardware symbols.
int main() { return 0; }
#endif
