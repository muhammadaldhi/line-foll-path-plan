#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include "config.h"
#include "models.h"
#include "menu.h"

struct MainScreenValues {
    int memory;
    int checkpoint;
    int stepPlan;
    int stopPlan;
};

struct PositionedValue {
    int x;
    int y;
    int value;
};

struct RenderPlan {
    const uint16_t *asset;
    SensorBarContainer sensorBox;
    uint16_t boxColor;
    PositionedValue memory;
    PositionedValue checkpoint;
    PositionedValue stepPlan;
    PositionedValue stopPlan;
};

class Display {
public:
    Display();
    void initialize();
    bool isInitialized() const;
    const uint16_t *asset(MenuPosition position) const;
    SensorBarContainer sensorBar() const;
    MainScreenText text() const;
    RenderPlan renderMainScreen(MenuPosition position, const uint16_t sensorValues[SENSOR_COUNT], const MainScreenValues& values);

private:
    bool initialized_;
};

#endif // DISPLAY_H
