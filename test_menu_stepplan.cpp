#include <cassert>
#include <cstdint>
#include <iostream>
#include "menu.h"
#include "models.h"

void testMenuStepPlanIntegration() {
    MenuState state = {MenuPosition::MAIN_1, 0, 0, 0, 0, 0, 0, 0};

    // 1. Navigation bounds: indexMemory (0..15), indexPlan (0..149), indexStopPlan (0..150)
    state.position = MenuPosition::MAIN_2;
    state.indexMemory = 15;
    processMenuInput(state, 3); // PLUS at boundary 15
    assert(state.indexMemory == 15);
    state.indexMemory = 0;
    processMenuInput(state, 4); // MINUS at boundary 0
    assert(state.indexMemory == 0);

    state.position = MenuPosition::MAIN_4;
    state.indexPlan = 149;
    processMenuInput(state, 3); // PLUS at boundary 149
    assert(state.indexPlan == 149);
    state.indexPlan = 0;
    processMenuInput(state, 4); // MINUS at boundary 0
    assert(state.indexPlan == 0);

    state.position = MenuPosition::MAIN_5;
    state.indexStopPlan = 150;
    processMenuInput(state, 3); // PLUS at boundary 150
    assert(state.indexStopPlan == 150);
    state.indexStopPlan = 0;
    processMenuInput(state, 4); // MINUS at boundary 0
    assert(state.indexStopPlan == 0);

    // 2. MAIN_3 checkpoint cursor covers configured checkpoints (0..29).
    state.position = MenuPosition::MAIN_3;
    state.indexCP = 28;
    processMenuInput(state, 3); // PLUS
    assert(state.indexCP == 29);
    processMenuInput(state, 3); // PLUS at maximum
    assert(state.indexCP == 29);
    state.indexCP = 0;
    processMenuInput(state, 4); // MINUS at minimum
    assert(state.indexCP == 0);

    uint32_t savedMemoryIndex = 0xFF;
    bool saveCalled = false;
    bool loadAllowed = true;
    MemoryEditorStorage storage;
    storage.load = [&](uint8_t index, MemoryProgram& mem) {
        if (!loadAllowed) return false;
        initializeDefaultMemory(mem);
        mem.stopPlan = 50 + index;
        return true;
    };
    storage.save = [&](uint8_t index, const MemoryProgram&) {
        savedMemoryIndex = index;
        saveCalled = true;
        return true;
    };
    configureMemoryEditor(storage, 0);

    // 3. MAIN_2 + ENTER loads selected memory before entering PLANNING.
    state.position = MenuPosition::MAIN_2;
    state.indexMemory = 5;
    processMenuInput(state, 5); // ENTER
    assert(state.position == MenuPosition::PLANNING);
    assert(activeMemory().stopPlan == 55);

    // Failed load must preserve active memory and current menu position.
    // Memory is index 5 currently. selectMemory(0) makes active memory 0.
    assert(selectMemory(0)); 
    activeMemory().stopPlan = 66;
    markMemoryDirty(1000);

    // Now indexMemory is 5. selectMemory(5) must fail.
    loadAllowed = false;
    state.position = MenuPosition::MAIN_2;
    state.indexMemory = 5;
    processMenuInput(state, 5); // ENTER
    assert(state.position == MenuPosition::MAIN_2);
    assert(activeMemory().stopPlan == 66);
    loadAllowed = true;

    // Reset to index 5 for BACK test
    selectMemory(5);

    // 4. BACK key exits PLANNING to MAIN_2 and calls editorExit/flush
    configureMemoryEditor(storage, 5);

    // Edit step parameter
    state.position = MenuPosition::PLANNING;
    saveCalled = false;
    editStepParameter(0, StepPlanParameter::RIGHT_SPEED, 88, 1000);
    editStepParameter(0, StepPlanParameter::KI, -12, 1000);
    assert(activeMemory().steps[0].pidKi == -12);
    editStepParameter(0, StepPlanParameter::KI, 256, 1000);
    assert(activeMemory().steps[0].pidKi == 255);
    editStepParameter(0, StepPlanParameter::KI, -256, 1000);
    assert(activeMemory().steps[0].pidKi == -255);
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_2);
    assert(saveCalled && savedMemoryIndex == 5);

    // 4. MAIN_3 + ENTER starts selected checkpoint / memory
    state.position = MenuPosition::MAIN_3;
    state.indexMemory = 3;
    state.indexCP = 12;
    processMenuInput(state, 5); // ENTER
    assert(state.position == MenuPosition::RUN_CHECKPOINT);
    assert(state.runMemory == 3);
    assert(state.runCheckpoint == 12);

    // 5. MAIN_5 + BACK opens Config screen
    state.position = MenuPosition::MAIN_5;
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::CONFIG);

    // 6. StopPlan edit updates memory and schedules save
    saveCalled = false;
    editStopPlan(120, 2000);
    assert(activeMemory().stopPlan == 120);
    assert(autosaveTick(2300)); // autosave triggers at 2000 + 300 ms
    assert(saveCalled);

    // 7. Verify uiMain.h rendering contracts & layout helpers
    auto bar = sensorBarContainer();
    assert(bar.x == 13 && bar.y == 6 && bar.width == 215 && bar.height == 37);
    auto text = mainScreenText();
    assert(text.memoryX == 12 && text.memoryY == 70);
    assert(text.checkpointX == 163 && text.checkpointY == 82);
    assert(text.planX == 8 && text.planY == 112);
    assert(text.stopPlanX == 11 && text.stopPlanY == 162);
    assert(mainScreenAsset(MenuPosition::MAIN_1) != nullptr);
    assert(mainScreenAsset(MenuPosition::MAIN_2) != nullptr);
    assert(mainScreenAsset(MenuPosition::MAIN_3) != nullptr);
    assert(mainScreenAsset(MenuPosition::MAIN_4) != nullptr);
    assert(mainScreenAsset(MenuPosition::MAIN_5) != nullptr);

    std::cout << "All menu stepplan integration tests passed!" << std::endl;
}

int main() {
    testMenuStepPlanIntegration();
    return 0;
}
