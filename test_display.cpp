#include <cassert>
#include <iostream>
#include "display.h"

int main() {
    Display display;
    assert(!display.isInitialized());
    display.initialize();
    assert(display.isInitialized());

    assert(display.asset(MenuPosition::MAIN_1) == main1);
    assert(display.asset(MenuPosition::MAIN_2) == main2);
    assert(display.asset(MenuPosition::MAIN_3) == main3);
    assert(display.asset(MenuPosition::MAIN_4) == main4);
    assert(display.asset(MenuPosition::MAIN_5) == main5);

    const auto bar = display.sensorBar();
    assert(bar.x == 13 && bar.y == 6 && bar.width == 215 && bar.height == 37);
    const auto text = display.text();
    assert(text.memoryX == 12 && text.memoryY == 70);
    assert(text.checkpointX == 163 && text.checkpointY == 82);
    assert(text.planX == 8 && text.planY == 112);
    assert(text.stopPlanX == 11 && text.stopPlanY == 162);
    const uint16_t sensors[SENSOR_COUNT] = {};
    MainScreenValues values{2, 7, 12, 99};
    const RenderPlan plan = display.renderMainScreen(MenuPosition::MAIN_2, sensors, values);
    assert(plan.asset == main2);
    assert(plan.boxColor == 0xED21);
    assert(plan.sensorBox.x == 13 && plan.sensorBox.y == 6);
    assert(plan.memory.x == 12 && plan.memory.y == 70 && plan.memory.value == 2);
    assert(plan.checkpoint.x == 163 && plan.checkpoint.y == 82 && plan.checkpoint.value == 7);
    assert(plan.stepPlan.x == 8 && plan.stepPlan.y == 112 && plan.stepPlan.value == 12);
    assert(plan.stopPlan.x == 11 && plan.stopPlan.y == 162 && plan.stopPlan.value == 99);

    assert(display.renderMainScreen(MenuPosition::MAIN_1, sensors, values).boxColor == 0xFFFF);
    std::cout << "All display tests passed!\n";
}
