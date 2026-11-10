#ifndef BUTTONS_H
#define BUTTONS_H

#include <array>
#include <cstdint>
#include <functional>
#include <cstddef>
#include "config.h"

enum class Button { NONE, UP, DOWN, PLUS, MINUS, ENTER, BACK };

struct AdcBand {
    uint16_t low;
    uint16_t high;
    Button button;
};

constexpr uint8_t BUTTON_BAND_COUNT = 3;
constexpr uint16_t BUTTON_ADC_MIN = 0;
constexpr uint16_t BUTTON_ADC_MAX = 4095;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;

struct ButtonConfig {
    ButtonConfig(
        std::array<AdcBand, BUTTON_BAND_COUNT> rBands = {{ {250, 450, Button::UP}, {900, 1150, Button::DOWN}, {1600, 1850, Button::PLUS} }},
        std::array<AdcBand, BUTTON_BAND_COUNT> lBands = {{ {250, 450, Button::MINUS}, {900, 1150, Button::ENTER}, {1600, 1850, Button::BACK} }},
        uint32_t debounce = BUTTON_DEBOUNCE_MS
    ) : rightBands(rBands), leftBands(lBands), debounceMs(debounce) {}

    std::array<AdcBand, BUTTON_BAND_COUNT> rightBands;
    std::array<AdcBand, BUTTON_BAND_COUNT> leftBands;
    uint32_t debounceMs;

    bool isValid() const {
        auto validateSide = [](const std::array<AdcBand, BUTTON_BAND_COUNT>& bands) {
            for (size_t i = 0; i < bands.size(); ++i) {
                if (bands[i].button == Button::NONE) continue;
                if (bands[i].low > bands[i].high || bands[i].high > BUTTON_ADC_MAX) return false;
                for (size_t j = i + 1; j < bands.size(); ++j) {
                    if (bands[j].button == Button::NONE) continue;
                    if (!(bands[i].high < bands[j].low || bands[i].low > bands[j].high)) {
                        return false;
                    }
                }
            }
            return true;
        };
        return validateSide(rightBands) && validateSide(leftBands);
    }
};

using ButtonAdcCallback = std::function<uint16_t(uint8_t)>;
using ButtonMillisCallback = std::function<uint32_t()>;

class ButtonReader {
public:
    explicit ButtonReader(ButtonAdcCallback adc, ButtonConfig config = {},
                          ButtonMillisCallback millis = {});
    Button read();
    bool isValid() const { return configValid_; }

private:
    Button decode(uint16_t value, const std::array<AdcBand, BUTTON_BAND_COUNT>& bands) const;
    ButtonAdcCallback adc_;
    ButtonMillisCallback millis_;
    ButtonConfig config_;
    bool configValid_ = true;
    Button stable_ = Button::NONE;
    Button candidate_ = Button::NONE;
    uint32_t candidateSince_ = 0;
};

#ifdef ARDUINO
void initializeButtons();
Button readButtons();
#endif

#endif
