#include <array>
#include <cassert>
#include <cstdint>
#include "sensors.h"

static void test_channel_mapping() {
    static_assert(SENSOR_CHANNEL_COUNT == 14);
    static_assert(channelForSensor(0) == 1);
    static_assert(channelForSensor(13) == 14);
    static_assert(selectChannel(channelForSensor(0)) == 1);
    static_assert(selectChannel(channelForSensor(13)) == 14);
    assert(channelForSensor(0) != 0);
    assert(channelForSensor(13) != 15);
}

static void test_reads_all_sensor_channels() {
    std::array<uint8_t, SENSOR_CHANNEL_COUNT> channels{};
    SensorReader reader([&](uint8_t channel) {
        channels[channel - 1] = channel;
        return static_cast<uint16_t>(100 + channel);
    });
    const auto values = reader.readRaw();
    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; ++i) {
        assert(channels[i] == channelForSensor(i));
        assert(values[i] == static_cast<uint16_t>(100 + channelForSensor(i)));
    }
}

static void test_calibration_and_active_mask() {
    assert(calibrationThreshold(100, 900, 0) == 500);
    assert(calibrationThreshold(100, 900, 50) == 250);
    assert(calibrationThreshold(100, 900, -50) == 750);
    const std::array<uint16_t, SENSOR_CHANNEL_COUNT> values = {100, 500, 501, 900, 0, 4095, 300, 700, 1, 2, 3, 4, 5, 6};
    const std::array<uint16_t, SENSOR_CHANNEL_COUNT> thresholds = {100, 500, 500, 1000, 1, 4095, 200, 700, 2, 2, 3, 5, 4, 7};
    assert(activeSensorMask(values, thresholds, LineMode::BLACK) == 0x1044);
    assert(activeSensorMask(values, thresholds, LineMode::WHITE) == 0x2fbb);
}

static void test_bar_levels() {
    assert(barLevel(100, 100) == 0);
    assert(barLevel(101, 100) == 0);
    assert(barLevel(550, 100) == 0);
    assert(barLevel(4095, 100) == 7);
    assert(barLevel(0, 100) == 0);
}

int main() {
    test_channel_mapping();
    test_reads_all_sensor_channels();
    test_calibration_and_active_mask();
    test_bar_levels();
}
