#ifndef SENSORS_H
#define SENSORS_H

#include <array>
#include <cstdint>
#include <functional>
#include "config.h"

enum class LineMode { BLACK, WHITE };
constexpr uint8_t SENSOR_CHANNEL_COUNT = SENSOR_COUNT;
constexpr uint8_t channelForSensor(uint8_t sensor) { return static_cast<uint8_t>(sensor + 1U); }
constexpr uint8_t selectChannel(uint8_t channel) { return channel; }

using SensorValues = std::array<uint16_t, SENSOR_CHANNEL_COUNT>;
using SensorReadCallback = std::function<uint16_t(uint8_t)>;

class SensorReader {
public:
    explicit SensorReader(SensorReadCallback callback);
    SensorValues readRaw() const;
private:
    SensorReadCallback callback_;
};

uint16_t calibrationThreshold(uint16_t low, uint16_t high, int8_t sensitivity);
uint16_t activeSensorMask(const SensorValues& values, const SensorValues& thresholds, LineMode mode);
uint8_t barLevel(uint16_t value, uint16_t threshold);

#ifdef ARDUINO
void initializeSensors();
SensorValues readSensors();
#endif

#endif
