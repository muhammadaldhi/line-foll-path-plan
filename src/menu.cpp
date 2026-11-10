#include "menu.h"
#include <algorithm>
#include "models.h"
#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "uiMain.h"
#else
// Host-test definitions; firmware supplies full 240x240 arrays from uiMain.h.
const uint16_t main1[] = {0};
const uint16_t main2[] = {0};
const uint16_t main3[] = {0};
const uint16_t main4[] = {0};
const uint16_t main5[] = {0};
#endif

// Input codes mapping: 1: UP, 2: DOWN, 3: PLUS, 4: MINUS, 5: ENTER, 6: BACK

namespace {
MemoryEditorStorage g_storage;
MemoryProgram g_memory;
uint8_t g_index = 0;
bool g_dirty = false;
uint32_t g_deadline = 0;
constexpr uint32_t kSaveDelay = 300;

constexpr int kMemoryMin = 0;
constexpr int kMemoryMax = MEMORY_COUNT - 1;
constexpr int kCPMin = 0;
constexpr int kCPMax = CHECKPOINT_COUNT - 1;
constexpr int kPlanMin = 0;
constexpr int kPlanMax = STEP_COUNT - 1;
constexpr int kStopPlanMin = 0;
constexpr int kStopPlanMax = STEP_COUNT;
}

void configureMemoryEditor(const MemoryEditorStorage& storage, uint8_t initialMemory) {
    g_storage = storage;
    g_index = initialMemory < MEMORY_COUNT ? initialMemory : 0;
    g_dirty = false;
    if (!g_storage.load || !g_storage.load(g_index, g_memory)) initializeDefaultMemory(g_memory);
}

MemoryProgram& activeMemory() { return g_memory; }

void markMemoryDirty(uint32_t nowMs) {
    g_dirty = true;
    g_deadline = nowMs + kSaveDelay;
}

bool autosaveTick(uint32_t nowMs) {
    if (!g_dirty || static_cast<uint32_t>(nowMs - (g_deadline - kSaveDelay)) < kSaveDelay) return false;
    return flushPendingSave();
}

bool flushPendingSave() {
    if (!g_dirty) return false;
    if (!g_storage.save || !g_storage.save(g_index, g_memory)) return false;
    g_dirty = false;
    return true;
}

bool selectMemory(uint8_t memoryIndex) {
    if (memoryIndex >= MEMORY_COUNT) return false;
    if (memoryIndex == g_index) return true;
    if (g_dirty && !flushPendingSave()) return false;
    MemoryProgram next;
    if (!g_storage.load || !g_storage.load(memoryIndex, next)) return false;
    g_memory = next;
    g_index = memoryIndex;
    return true;
}

void editorExit() { flushPendingSave(); }

void editStepParameter(uint16_t stepIndex, StepPlanParameter param, int value, uint32_t nowMs) {
    if (stepIndex >= STEP_COUNT) return;
    StepPlan& step = g_memory.steps[stepIndex];
    switch (param) {
        case StepPlanParameter::ACTION:
            step.action = static_cast<Action>(value);
            break;
        case StepPlanParameter::SENSOR:
            step.sensor = static_cast<Sensor>(value);
            break;
        case StepPlanParameter::RIGHT_SPEED:
            step.rightSpeed = static_cast<int16_t>(value);
            break;
        case StepPlanParameter::LEFT_SPEED:
            step.leftSpeed = static_cast<int16_t>(value);
            break;
        case StepPlanParameter::COUNTER_MODE:
            step.counterMode = static_cast<CounterMode>(value);
            break;
        case StepPlanParameter::COUNTER:
            step.counter = static_cast<uint32_t>(value);
            break;
        case StepPlanParameter::VALUE_A:
            step.valueA = static_cast<int32_t>(value);
            break;
        case StepPlanParameter::COUNTER_A:
            step.counterA = static_cast<int32_t>(value);
            break;
        case StepPlanParameter::COUNTER_A_MODE:
            step.counterAMode = static_cast<CounterMode>(value);
            break;
        case StepPlanParameter::VALUE_B:
            step.valueB = static_cast<int32_t>(value);
            break;
        case StepPlanParameter::COUNTER_B:
            step.counterB = static_cast<int32_t>(value);
            break;
        case StepPlanParameter::COUNTER_B_MODE:
            step.counterBMode = static_cast<CounterMode>(value);
            break;
        case StepPlanParameter::PID_PROFILE:
            step.pidProfile = static_cast<uint8_t>(value);
            break;
        case StepPlanParameter::KI:
            step.pidKi = static_cast<int16_t>(std::clamp(value, -255, 255));
            break;
        case StepPlanParameter::FOLLOW_MODE:
            step.followMode = static_cast<FollowMode>(value);
            break;
        case StepPlanParameter::JUMP_ENABLED:
            step.jumpEnabled = (value != 0);
            break;
        case StepPlanParameter::JUMP_TARGET_MEMORY:
            step.jumpTargetMemory = static_cast<uint8_t>(value);
            break;
        case StepPlanParameter::JUMP_TARGET_STEP:
            step.jumpTargetStep = static_cast<uint16_t>(value);
            break;
        case StepPlanParameter::STOP_PLAN:
        case StepPlanParameter::COUNT:
            break;
    }
    markMemoryDirty(nowMs);
}

void editStopPlan(uint16_t stopPlan, uint32_t nowMs) {
    if (stopPlan > STEP_COUNT) return;
    g_memory.stopPlan = stopPlan;
    markMemoryDirty(nowMs);
}

SensorBarContainer sensorBarContainer() {
    return {13, 6, 215, 37};
}

MainScreenText mainScreenText() {
    return {12, 70, 163, 82, 8, 112, 11, 162};
}

const uint16_t *mainScreenAsset(MenuPosition position) {
    switch (position) {
        case MenuPosition::MAIN_1: return main1;
        case MenuPosition::MAIN_2: return main2;
        case MenuPosition::MAIN_3: return main3;
        case MenuPosition::MAIN_4: return main4;
        case MenuPosition::MAIN_5: return main5;
        default: return nullptr;
    }
}

void processMenuInput(MenuState &state, int input) {
    switch (state.position) {
        case MenuPosition::MAIN_1:
            if (input == 1) state.position = MenuPosition::MAIN_5;
            else if (input == 2) state.position = MenuPosition::MAIN_2;
            else if (input == 5) { /* Enter sensor config */ }
            break;
        case MenuPosition::MAIN_2:
            if (input == 1) state.position = MenuPosition::MAIN_1;
            else if (input == 2) state.position = MenuPosition::MAIN_3;
            else if (input == 5) {
                if (selectMemory(static_cast<uint8_t>(state.indexMemory))) state.position = MenuPosition::PLANNING;
            }
            else if (input == 3) { if (state.indexMemory < kMemoryMax) state.indexMemory++; }
            else if (input == 4) { if (state.indexMemory > kMemoryMin) state.indexMemory--; }
            break;
        case MenuPosition::MAIN_3:
            if (input == 1) state.position = MenuPosition::MAIN_2;
            else if (input == 2) state.position = MenuPosition::MAIN_5;
            else if (input == 3) { if (state.indexCP < kCPMax) state.indexCP++; }
            else if (input == 4) { if (state.indexCP > kCPMin) state.indexCP--; }
            else if (input == 5) { state.runMemory = state.indexMemory; state.runCheckpoint = state.indexCP; state.position = MenuPosition::RUN_CHECKPOINT; }
            else if (input == 6) state.position = MenuPosition::MAIN_4;
            break;
        case MenuPosition::MAIN_4:
            if (input == 1) state.position = MenuPosition::MAIN_3;
            else if (input == 2) state.position = MenuPosition::MAIN_5;
            else if (input == 3) { if (state.indexPlan < kPlanMax) state.indexPlan++; }
            else if (input == 4) { if (state.indexPlan > kPlanMin) state.indexPlan--; }
            break;
        case MenuPosition::MAIN_5:
            if (input == 1) state.position = MenuPosition::MAIN_4;
            else if (input == 2) state.position = MenuPosition::MAIN_1;
            else if (input == 3) { if (state.indexStopPlan < kStopPlanMax) state.indexStopPlan++; }
            else if (input == 4) { if (state.indexStopPlan > kStopPlanMin) state.indexStopPlan--; }
            else if (input == 6) { state.position = MenuPosition::CONFIG; }
            break;
        case MenuPosition::PLANNING:
            if (input == 1 && state.indexPlan < kPlanMax) ++state.indexPlan;
            else if (input == 2 && state.indexPlan > kPlanMin) --state.indexPlan;
            else if (input == 3 && state.selectedParameter < static_cast<int>(StepPlanParameter::COUNT) - 1) ++state.selectedParameter;
            else if (input == 4 && state.selectedParameter > 0) --state.selectedParameter;
            else if (input == 6) { editorExit(); state.position = MenuPosition::MAIN_2; }
            break;
        case MenuPosition::RUN_CHECKPOINT:
            if (input == 6) state.position = MenuPosition::MAIN_3;
            break;
        case MenuPosition::CONFIG:
            if (input == 6) state.position = MenuPosition::MAIN_5;
            break;
    }
}