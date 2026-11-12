#include "planner.h"

#ifdef ARDUINO
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    alwi.begin();
}

void loop() {
    alwi.loop();
}

#else
int main() { return 0; }
#endif
