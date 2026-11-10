#ifndef BUZZER_H
#define BUZZER_H

#include <cstdint>
#include <functional>
#include "config.h"

struct BuzzerEvent {
    uint16_t frequency;
    uint16_t durationMs;
    bool active;
};

using BuzzerCallback = std::function<void(int8_t, uint16_t, uint16_t)>;

class BuzzerController {
public:
    BuzzerController(BuzzerCallback callback, int8_t pin = BUZZER_PIN);
    void beep(uint16_t frequency, uint16_t durationMs);
    void stop();
    BuzzerEvent lastEvent() const;

private:
    BuzzerCallback callback_;
    int8_t pin_;
    BuzzerEvent lastEvent_{};
};

#endif
