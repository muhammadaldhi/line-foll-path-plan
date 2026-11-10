#ifndef MENU_H
#define MENU_H

#include <cstdint>
#include <functional>

struct MemoryProgram;

// UI bitmap assets are 240x240 RGB565 images defined by uiMain.h.
extern const uint16_t main1[];
extern const uint16_t main2[];
extern const uint16_t main3[];
extern const uint16_t main4[];
extern const uint16_t main5[];

enum class Action {
    NONE,
    MOVE_FWD,
    MOVE_BWD,
    TURN_RGT,
    TURN_LFT,
    PICK,
    DROP,
    BLOW,
    FINISH
};

enum class MenuPosition {
    MAIN_1,
    MAIN_2,
    MAIN_3,
    MAIN_4,
    MAIN_5,
    PLANNING,
    RUN_CHECKPOINT,
    CONFIG
};

enum class MenuScreen {
    MAIN_1,
    MAIN_2,
    MAIN_3,
    MAIN_4,
    MAIN_5
};

enum class StepPlanParameter : uint8_t {
    ACTION, SENSOR, RIGHT_SPEED, LEFT_SPEED, COUNTER_MODE, COUNTER,
    VALUE_A, COUNTER_A, COUNTER_A_MODE, VALUE_B, COUNTER_B, COUNTER_B_MODE,
    PID_PROFILE, KI, FOLLOW_MODE, JUMP_ENABLED, JUMP_TARGET_MEMORY,
    JUMP_TARGET_STEP, STOP_PLAN, COUNT
};

struct MenuState {
    MenuPosition position;
    int indexMemory;
    int indexCP;
    int indexPlan;
    int indexStopPlan;
    int selectedParameter = 0;
    int runMemory = 0;
    int runCheckpoint = 0;
};

struct MemoryEditorStorage {
    std::function<bool(uint8_t, MemoryProgram&)> load;
    std::function<bool(uint8_t, const MemoryProgram&)> save;
};

void configureMemoryEditor(const MemoryEditorStorage& storage, uint8_t initialMemory = 0);
MemoryProgram& activeMemory();
void markMemoryDirty(uint32_t nowMs);
bool autosaveTick(uint32_t nowMs);
bool flushPendingSave();
bool selectMemory(uint8_t memoryIndex);
void editorExit();

// Apply editor changes and schedule coalesced save.
void editStepParameter(uint16_t stepIndex, StepPlanParameter param, int value, uint32_t nowMs);
void editStopPlan(uint16_t stopPlan, uint32_t nowMs);

void processMenuInput(MenuState &state, int input);

// Host-testable screen layout for 240x240 UI. Coordinates use pixels.
struct SensorBarContainer {
    int x;
    int y;
    int width;
    int height;
};

struct MainScreenText {
    int memoryX;
    int memoryY;
    int checkpointX;
    int checkpointY;
    int planX;
    int planY;
    int stopPlanX;
    int stopPlanY;
};

SensorBarContainer sensorBarContainer();
MainScreenText mainScreenText();
const uint16_t *mainScreenAsset(MenuPosition position);

#endif