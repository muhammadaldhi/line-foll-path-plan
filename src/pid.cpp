#include "pid.h"

#include <algorithm>
#include <array>

std::array<PIDProfile, 5> PIDController::defaultProfiles() {
    return {{{10, 0, 20}, {13, 0, 25}, {20, 0, 50}, {10, 0, 20}, {10, 0, 20}}};
}

PIDController::PIDController(std::array<PIDProfile, 5> profiles) : profiles_(profiles) {}

bool PIDController::setProfile(uint8_t profile, PIDProfile values) {
    if (profile >= profiles_.size()) return false;
    profiles_[profile] = values;
    return true;
}

PIDProfile PIDController::profile(uint8_t profile) const {
    return profiles_[profile < profiles_.size() ? profile : 0];
}

void PIDController::reset() {
    integral_ = 0;
    lastError_ = 0;
    hasLastError_ = false;
    hasTimedUpdate_ = false;
}

MotorCommand PIDController::update(uint16_t activeMask, FollowMode mode, uint8_t profile, uint8_t baseSpeed) {
    const PIDProfile& p = profiles_[profile < profiles_.size() ? profile : 0];
    return update(activeMask, mode, profile, baseSpeed, p.ki);
}

MotorCommand PIDController::update(uint16_t activeMask, FollowMode mode, uint8_t profile, uint8_t baseSpeed, int16_t kiOverride) {
    constexpr std::array<int16_t, SENSOR_COUNT> center{{-26,-22,-18,-14,-10,-5,-1,1,5,10,14,18,22,26}};
    constexpr std::array<int16_t, SENSOR_COUNT> right{{-46,-42,-38,-34,-30,-26,-22,-18,-14,-10,-5,-1,1,5}};
    constexpr std::array<int16_t, SENSOR_COUNT> left{{-5,-1,1,5,10,14,18,22,26,30,34,38,42,46}};
    const uint16_t mask = static_cast<uint16_t>(activeMask & 0x3fff);
    if (mask == 0) {
        reset();
        return {0, 0, true};
    }
    if (mode == FollowMode::NONE) mode = FollowMode::CENTER;
    const auto& weights = mode == FollowMode::RIGHT ? right : mode == FollowMode::LEFT ? left : center;
    int32_t sum = 0;
    uint8_t count = 0;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        if ((mask & (uint16_t{1} << i)) != 0) { sum += weights[i]; ++count; }
    }
    const int32_t error = sum / count;
    integral_ += error;
    const PIDProfile& p = profiles_[profile < profiles_.size() ? profile : 0];
    const int32_t integralContribution = std::clamp<int32_t>(integral_ * static_cast<int32_t>(kiOverride), -100, 100);
    const int32_t derivative = error - lastError_;
    const int32_t moveVal = error * p.kp + integralContribution + derivative * p.kd;
    lastError_ = error;
    hasLastError_ = true;
    const int32_t speed = static_cast<int32_t>(std::min<uint32_t>(baseSpeed, 100) * 255u / 100u);
    const auto clampSpeed = [](int32_t value) { return static_cast<int16_t>(std::clamp<int32_t>(value, -255, 255)); };
    return {clampSpeed(speed + moveVal), clampSpeed(speed - moveVal), false};
}

MotorCommand PIDController::update(uint16_t activeMask, FollowMode mode, uint8_t profile,
                                   uint8_t baseSpeed, uint32_t nowMs, uint32_t updateIntervalMs) {
    if (hasTimedUpdate_ && static_cast<uint32_t>(nowMs - lastUpdateMs_) < updateIntervalMs) {
        return previousCommand_;
    }
    previousCommand_ = update(activeMask, mode, profile, baseSpeed);
    lastUpdateMs_ = nowMs;
    hasTimedUpdate_ = true;
    return previousCommand_;
}
