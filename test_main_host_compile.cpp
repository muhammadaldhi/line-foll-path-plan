// Compile-contract test for entrypoint-owned host-visible API wiring.
#include <type_traits>
#include "display.h"
#include "menu.h"
#include "sensors.h"
#include "storage.h"
#include "motors.h"
#include "pid.h"
#include "planner.h"

static_assert(std::is_same_v<decltype(&initializeStorage), bool (*)(const char*)>);
static_assert(std::is_same_v<decltype(&loadMemory), bool (*)(uint8_t, MemoryProgram&)>);
static_assert(std::is_same_v<decltype(&saveMemory), bool (*)(uint8_t, const MemoryProgram&)>);
static_assert(std::is_same_v<decltype(&processMenuInput), void (*)(MenuState&, int)>);
static_assert(std::is_same_v<decltype(&Display::initialize), void (Display::*)()>);
static_assert(std::is_same_v<decltype(&Display::renderMainScreen),
    RenderPlan (Display::*)(MenuPosition, const uint16_t[SENSOR_COUNT], const MainScreenValues&)>);
static_assert(std::is_same_v<decltype(&SensorReader::readRaw), SensorValues (SensorReader::*)() const>);
static_assert(std::is_same_v<decltype(&MotorController::drive), void (MotorController::*)(int16_t, int16_t)>);
using TimedPidUpdate = MotorCommand (PIDController::*)(
    uint16_t, FollowMode, uint8_t, uint8_t, uint32_t, uint32_t);
static_assert(std::is_same_v<decltype(static_cast<TimedPidUpdate>(&PIDController::update)),
    TimedPidUpdate>);
static_assert(std::is_same_v<decltype(&PlannerRuntime::step),
    RuntimeResult (PlannerRuntime::*)(const MemoryProgram&, const RuntimeInput&)>);

int main() { return 0; }
