#include <cassert>
#include <cstdint>
#include <vector>
#include "buzzer.h"

struct Call {
    int8_t pin;
    uint16_t frequency;
    uint16_t durationMs;
};

int main() {
    std::vector<Call> calls;
    BuzzerController buzzer(
        [&calls](int8_t pin, uint16_t frequency, uint16_t durationMs) {
            calls.push_back({pin, frequency, durationMs});
        }, 7);

    buzzer.beep(440, 250);
    const BuzzerEvent beep = buzzer.lastEvent();
    assert(beep.frequency == 440);
    assert(beep.durationMs == 250);
    assert(beep.active);
    assert(calls.size() == 1);
    assert(calls[0].pin == 7 && calls[0].frequency == 440 && calls[0].durationMs == 250);

    buzzer.stop();
    const BuzzerEvent stopped = buzzer.lastEvent();
    assert(stopped.frequency == 0);
    assert(stopped.durationMs == 0);
    assert(!stopped.active);
    assert(calls.size() == 2);
    assert(calls[1].pin == 7 && calls[1].frequency == 0 && calls[1].durationMs == 0);

    calls.clear();
    BuzzerController disconnected(
        [&calls](int8_t pin, uint16_t frequency, uint16_t durationMs) {
            calls.push_back({pin, frequency, durationMs});
        });
    disconnected.beep(880, 100);
    assert(disconnected.lastEvent().active);
    disconnected.stop();
    assert(!disconnected.lastEvent().active);
    assert(calls.empty());
}
