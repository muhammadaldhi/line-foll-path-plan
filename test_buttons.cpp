#include <cassert>
#include <cstdint>
#include "buttons.h"

static void test_decodes_configured_ladders() {
    ButtonConfig config{};
    config.rightBands = {{{100, 199, Button::UP}, {300, 399, Button::DOWN},
                          {500, 599, Button::PLUS}}};
    config.leftBands = {{{100, 199, Button::MINUS}, {300, 399, Button::ENTER},
                         {500, 599, Button::BACK}}};
    config.debounceMs = 0;
    uint16_t right = 150;
    uint16_t left = 350;
    ButtonReader reader([&](uint8_t channel) { return channel == 0 ? right : left; }, config, [](){return 1000;});
    assert(reader.read() == Button::UP);
    right = 0;
    assert(reader.read() == Button::ENTER);
    left = 4095;
    assert(reader.read() == Button::NONE);
}

static void test_debounce_requires_stable_interval() {
    ButtonConfig config{};
    config.debounceMs = 50;
    config.rightBands = {{{100, 199, Button::UP}}};
    uint32_t now = 0;
    uint16_t adc = 150;
    ButtonReader reader([&](uint8_t) { return adc; }, config, [&]() { return now; });
    assert(reader.read() == Button::NONE);
    now = 49;
    assert(reader.read() == Button::NONE);
    now = 50;
    assert(reader.read() == Button::UP);
}

static void test_rejects_invalid_bands() {
    ButtonConfig reversed{};
    reversed.rightBands[0] = {500, 100, Button::UP};
    ButtonReader reversedReader([](uint8_t) { return uint16_t{150}; }, reversed);
    assert(!reversed.isValid());
    assert(!reversedReader.isValid());
    assert(reversedReader.read() == Button::NONE);

    ButtonConfig overlap{};
    overlap.rightBands[1] = {400, 500, Button::DOWN};
    ButtonReader overlapReader([](uint8_t) { return uint16_t{450}; }, overlap);
    assert(!overlap.isValid());
    assert(overlapReader.read() == Button::NONE);
}

static void test_selects_only_c0_and_c15() {
    ButtonConfig config{};
    uint8_t selected = 99;
    ButtonReader reader([&](uint8_t channel) { selected = channel; return uint16_t{0}; }, config);
    reader.read();
    assert(selected == 15);
}

int main() {
    test_decodes_configured_ladders();
    test_debounce_requires_stable_interval();
    test_rejects_invalid_bands();
    test_selects_only_c0_and_c15();
}
