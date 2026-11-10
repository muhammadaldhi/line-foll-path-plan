#include "buttons.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

ButtonReader::ButtonReader(ButtonAdcCallback adc, ButtonConfig config, ButtonMillisCallback millis)
    : adc_(adc), millis_(millis), config_(config), configValid_(config.isValid()) {
    if (!millis_) millis_ = []() { return uint32_t{0}; };
}

Button ButtonReader::decode(uint16_t value, const std::array<AdcBand, BUTTON_BAND_COUNT>& bands) const {
    for (const AdcBand& band : bands) {
        if (value >= band.low && value <= band.high) return band.button;
    }
    return Button::NONE;
}

Button ButtonReader::read() {
    if (!configValid_) return Button::NONE;
    const Button right = decode(adc_ ? adc_(0) : 0, config_.rightBands);
    const Button left = decode(adc_ ? adc_(15) : 0, config_.leftBands);
    const Button current = right != Button::NONE ? right : left;
    const uint32_t now = millis_();
    if (current != candidate_) {
        candidate_ = current;
        candidateSince_ = now;
        if (config_.debounceMs == 0) stable_ = current;
    } else if (current != stable_ && now - candidateSince_ >= config_.debounceMs) {
        stable_ = current;
    }
    return stable_;
}

#ifdef ARDUINO
static void selectButtonChannel(uint8_t channel) {
    digitalWrite(SEL0, (channel & 1U) ? HIGH : LOW);
    digitalWrite(SEL1, (channel & 2U) ? HIGH : LOW);
    digitalWrite(SEL2, (channel & 4U) ? HIGH : LOW);
    digitalWrite(SEL3, (channel & 8U) ? HIGH : LOW);
}

void initializeButtons() {
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);
    pinMode(SEL2, OUTPUT);
    pinMode(SEL3, OUTPUT);
    pinMode(OUT_MUX, INPUT);
}

Button readButtons() {
    static ButtonReader reader([](uint8_t channel) {
        selectButtonChannel(channel);
        return static_cast<uint16_t>(analogRead(OUT_MUX));
    }, ButtonConfig{}, []() { return millis(); });
    return reader.read();
}
#endif
