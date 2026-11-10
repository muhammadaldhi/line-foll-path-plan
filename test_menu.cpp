#include <iostream>
#include <cassert>
#include "menu.h"

void testMenuNavigation() {
    MenuState state = { MenuPosition::MAIN_1, 0, 0, 0, 0 };

    // Navigation down MAIN_1 -> MAIN_2 -> MAIN_3 -> MAIN_5 -> MAIN_1
    processMenuInput(state, 2); // DOWN
    assert(state.position == MenuPosition::MAIN_2);

    processMenuInput(state, 2); // DOWN
    assert(state.position == MenuPosition::MAIN_3);

    processMenuInput(state, 2); // DOWN
    assert(state.position == MenuPosition::MAIN_5);

    processMenuInput(state, 2); // DOWN
    assert(state.position == MenuPosition::MAIN_1);

    // Navigation UP from MAIN_1 -> MAIN_5 -> MAIN_4 -> MAIN_3
    processMenuInput(state, 1); // UP
    assert(state.position == MenuPosition::MAIN_5);

    processMenuInput(state, 1); // UP
    assert(state.position == MenuPosition::MAIN_4);

    processMenuInput(state, 1); // UP
    assert(state.position == MenuPosition::MAIN_3);

    // BACK key from MAIN_3 -> MAIN_4 (overwrites PLANNING)
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_4);
    
    // ENTER key from MAIN_2 -> PLANNING
    state.position = MenuPosition::MAIN_2;
    processMenuInput(state, 5); // ENTER
    assert(state.position == MenuPosition::PLANNING);

    // BACK key from PLANNING -> MAIN_2
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_2);

    // ENTER key from MAIN_3 -> RUN_CHECKPOINT
    state.position = MenuPosition::MAIN_3;
    processMenuInput(state, 5); // ENTER
    assert(state.position == MenuPosition::RUN_CHECKPOINT);

    // BACK key from MAIN_3 -> MAIN_4
    state.position = MenuPosition::MAIN_3;
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_4);

    // BACK key from MAIN_5 -> CONFIG
    state.position = MenuPosition::MAIN_5;
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::CONFIG);
    
    // BACK key from CONFIG -> MAIN_5
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_5);

    // Value modification testing in MAIN_2
    state.position = MenuPosition::MAIN_2;
    state.indexMemory = 0;
    processMenuInput(state, 3); // PLUS
    assert(state.indexMemory == 1);
    processMenuInput(state, 4); // MINUS
    assert(state.indexMemory == 0);

    // Bounds checking for indexMemory (0-15)
    state.indexMemory = 15;
    processMenuInput(state, 3); // PLUS
    assert(state.indexMemory == 15);
    state.indexMemory = 0;
    processMenuInput(state, 4); // MINUS
    assert(state.indexMemory == 0);

    // Bounds checking for indexCP (0-29) in MAIN_3
    state.position = MenuPosition::MAIN_3;
    state.indexCP = 29;
    processMenuInput(state, 3); // PLUS
    assert(state.indexCP == 29);
    state.indexCP = 0;
    processMenuInput(state, 4); // MINUS
    assert(state.indexCP == 0);

    // Bounds checking for indexPlan (0-149) in MAIN_4
    state.position = MenuPosition::MAIN_4;
    state.indexPlan = 149;
    processMenuInput(state, 3); // PLUS
    assert(state.indexPlan == 149);
    state.indexPlan = 0;
    processMenuInput(state, 4); // MINUS
    assert(state.indexPlan == 0);

    // Bounds checking for indexStopPlan (0-150) in MAIN_5
    state.position = MenuPosition::MAIN_5;
    state.indexStopPlan = 150;
    processMenuInput(state, 3); // PLUS
    assert(state.indexStopPlan == 150);
    state.indexStopPlan = 0;
    processMenuInput(state, 4); // MINUS
    assert(state.indexStopPlan == 0);

    // Config screen navigation test: from MAIN_5 BACK -> CONFIG
    state.position = MenuPosition::MAIN_5;
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::CONFIG);

    // Config screen navigation test: BACK from CONFIG -> MAIN_5
    processMenuInput(state, 6); // BACK
    assert(state.position == MenuPosition::MAIN_5);

    // Layout helpers test
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

    std::cout << "All menu navigation tests passed!" << std::endl;
}

int main() {
    testMenuNavigation();
    return 0;
}