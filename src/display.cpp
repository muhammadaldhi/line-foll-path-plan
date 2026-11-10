#include "display.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "uiMain.h"

namespace {
TFT_eSPI tft;

void drawValue(const PositionedValue& value, uint16_t color) {
    tft.setTextColor(color);
    tft.setCursor(value.x, value.y);
    tft.print(value.value);
}

void drawPlan(const RenderPlan& plan, const uint16_t sensorValues[SENSOR_COUNT]) {
    if (plan.asset == nullptr) return;

    tft.pushImage(0, 0, 240, 240, const_cast<uint16_t*>(plan.asset));
    tft.drawRect(plan.sensorBox.x, plan.sensorBox.y, plan.sensorBox.width, plan.sensorBox.height, plan.boxColor);
    for (uint8_t index = 0; index < SENSOR_COUNT; ++index) {
        const int x = plan.sensorBox.x + 4 + static_cast<int>(index) * 15;
        tft.fillRect(x, plan.sensorBox.y + 4, 10, 29, sensorValues[index] == 0 ? TFT_BLACK : TFT_WHITE);
    }
    drawValue(plan.memory, plan.boxColor);
    drawValue(plan.checkpoint, plan.boxColor);
    drawValue(plan.stepPlan, plan.boxColor);
    drawValue(plan.stopPlan, plan.boxColor);
}
} // namespace
#endif

Display::Display() : initialized_(false) {}

void Display::initialize() {
#ifdef ARDUINO
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
#endif
    initialized_ = true;
}

bool Display::isInitialized() const {
    return initialized_;
}

const uint16_t *Display::asset(MenuPosition position) const {
    return mainScreenAsset(position);
}

SensorBarContainer Display::sensorBar() const {
    return sensorBarContainer();
}

MainScreenText Display::text() const {
    return mainScreenText();
}

RenderPlan Display::renderMainScreen(MenuPosition position, const uint16_t sensorValues[SENSOR_COUNT], const MainScreenValues& values) {
    const MainScreenText positions = text();
    RenderPlan plan{
        asset(position),
        sensorBar(),
        static_cast<uint16_t>(position == MenuPosition::MAIN_1 ? 0xFFFF : 0xED21),
        {positions.memoryX, positions.memoryY, values.memory},
        {positions.checkpointX, positions.checkpointY, values.checkpoint},
        {positions.planX, positions.planY, values.stepPlan},
        {positions.stopPlanX, positions.stopPlanY, values.stopPlan}
    };
#ifdef ARDUINO
    if (initialized_) drawPlan(plan, sensorValues);
#else
    (void)sensorValues;
#endif
    return plan;
}
