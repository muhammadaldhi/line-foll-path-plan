#include "sensors.h"
#include <algorithm>

#ifdef ARDUINO
#include <Arduino.h>
#endif

SensorReader::SensorReader(SensorReadCallback callback) : callback_(callback) {}

SensorValues SensorReader::readRaw() const {
    SensorValues values{};
    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; ++i) {
        if (callback_) {
            values[i] = callback_(channelForSensor(i));
        } else {
            values[i] = 0;
        }
    }
    return values;
}

uint16_t calibrationThreshold(uint16_t low, uint16_t high, int8_t sensitivity) {
    int32_t midpoint = static_cast<int32_t>(low) + (static_cast<int32_t>(high) - static_cast<int32_t>(low)) / 2;
    int32_t adjustment = (midpoint * sensitivity) / 100;
    int32_t result = midpoint - adjustment;
    if (result < 0) return 0;
    if (result > 4095) return 4095;
    return static_cast<uint16_t>(result);
}

uint16_t activeSensorMask(const SensorValues& values, const SensorValues& thresholds, LineMode mode) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; ++i) {
        if (values[i] > thresholds[i]) {
            mask |= static_cast<uint16_t>(1U << i);
        }
    }
    if (mode == LineMode::WHITE) {
        mask = static_cast<uint16_t>(~mask & 0x3fff);
    }
    return mask;
}

uint8_t barLevel(uint16_t value, uint16_t threshold) {
    if (value <= threshold) return 0;
    uint32_t high = std::min(static_cast<uint32_t>(value), static_cast<uint32_t>(4095));
    uint32_t thresh = std::min(static_cast<uint32_t>(threshold), static_cast<uint32_t>(4095));
    if (high <= thresh) return 0;
    uint32_t scaled = (high - thresh) * 7 / (4095 - thresh > 0 ? (4095 - thresh) : 1);
    if (scaled > 7) return 7;
    return static_cast<uint8_t>(scaled);
}

#ifdef ARDUINO
static void switchMuxChannel(uint8_t channel) {
    digitalWrite(SEL0, (channel & 1) ? HIGH : LOW);
    digitalWrite(SEL1, (channel & 2) ? HIGH : LOW);
    digitalWrite(SEL2, (channel & 4) ? HIGH : LOW);
    digitalWrite(SEL3, (channel & 8) ? HIGH : LOW);
}

void initializeSensors() {
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);
    pinMode(SEL2, OUTPUT);
    pinMode(SEL3, OUTPUT);
    pinMode(OUT_MUX, INPUT);
}

SensorValues readSensors() {
    SensorValues values{};
    for (uint8_t i = 0; i <= 6; ++i) {
        const uint8_t rightSensor = i;
        const uint8_t leftSensor = static_cast<uint8_t>(13 - i);
        switchMuxChannel(channelForSensor(rightSensor));
        delayMicroseconds(20);
        values[rightSensor] = analogRead(OUT_MUX);
        
        switchMuxChannel(channelForSensor(leftSensor));
        delayMicroseconds(20);
        values[leftSensor] = analogRead(OUT_MUX);
    }
    return values;
}
#endif
