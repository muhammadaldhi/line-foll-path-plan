#ifndef PID_H
#define PID_H

#include <array>
#include <cstdint>
#include "models.h"

struct PIDProfile { int16_t kp; int16_t ki; int16_t kd; };
struct MotorCommand { int16_t rightSpeed; int16_t leftSpeed; bool lineLost; };

class PIDController {
public:
    static std::array<PIDProfile, 5> defaultProfiles();
    explicit PIDController(std::array<PIDProfile, 5> profiles = defaultProfiles());
    MotorCommand update(uint16_t activeMask, FollowMode mode, uint8_t profile, uint8_t baseSpeed);
    MotorCommand update(uint16_t activeMask, FollowMode mode, uint8_t profile, uint8_t baseSpeed, int16_t kiOverride);
    MotorCommand update(uint16_t activeMask, FollowMode mode, uint8_t profile, uint8_t baseSpeed,
                        uint32_t nowMs, uint32_t updateIntervalMs);
    void reset();
    bool setProfile(uint8_t profile, PIDProfile values);
    PIDProfile profile(uint8_t profile) const;
private:
    std::array<PIDProfile, 5> profiles_;
    int32_t integral_ = 0;
    int32_t lastError_ = 0;
    bool hasLastError_ = false;
    MotorCommand previousCommand_{0, 0, false};
    uint32_t lastUpdateMs_ = 0;
    bool hasTimedUpdate_ = false;
};

#endif
