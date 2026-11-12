#include "planner.h"

#ifdef ARDUINO
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    alwi.begin();

    alwi.pid(1, 0.8f, 10.0f, 1.0f)
        .pid(2, 2.0f, 10.0f, 1.0f)
        .pid(3, 6.0f, 11.0f, 3.0f)
        .pid(4, 2.0f, 0.0f, 4.0f)
        .memory(0)
        .step(1)
        .forward(SensorLogic::XOR_CENTER_LEFT_1, 180, 180, 50)
        .afterForward(225, 20)
        .usePid(1)
        .lineMode(LineColor::BLACK)
        .stopAfterAction()
        .save();
}

void loop() {
    alwi.loop();
}

#else
int main() { return 0; }
#endif
