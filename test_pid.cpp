#include <cassert>
#include <array>
#include <cstdint>
#include "pid.h"

static void test_centered_and_lost() {
    PIDController c;
    auto centered = c.update(0x3fff, FollowMode::CENTER, 0, 50);
    assert(centered.rightSpeed == centered.leftSpeed);
    assert(!centered.lineLost);
    auto lost = c.update(0, FollowMode::CENTER, 0, 50);
    assert(lost.lineLost);
    assert(lost.rightSpeed == 0 && lost.leftSpeed == 0);
}
static void test_weights_and_none() {
    PIDController c;
    auto right = c.update(1u << 13, FollowMode::CENTER, 0, 50);
    assert(right.rightSpeed != right.leftSpeed);
    c.reset();
    auto none = c.update(1u << 13, FollowMode::NONE, 0, 50);
    auto explicitCenter = PIDController().update(1u << 13, FollowMode::CENTER, 0, 50);
    std::array<PIDProfile, 3> noTerms{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
    PIDController kiController(noTerms);
    kiController.update(1u << 13, FollowMode::CENTER, 0, 10, 0);
    auto baseline = kiController.update(1u << 13, FollowMode::CENTER, 0, 10, 0);
    PIDController overrideController(noTerms);
    overrideController.update(1u << 13, FollowMode::CENTER, 0, 10, 0);
    auto kiOverride = overrideController.update(1u << 13, FollowMode::CENTER, 0, 10, 17);
    assert(kiOverride.rightSpeed != baseline.rightSpeed);
    assert(none.rightSpeed == explicitCenter.rightSpeed && none.leftSpeed == explicitCenter.leftSpeed);
}
static void test_pid_terms_profiles_and_reset() {
    std::array<PIDProfile, 3> p{{{0, 1, 0}, {10, 0, 0}, {0, 0, 10}}};
    PIDController c(p);
    auto first = c.update(1u << 6, FollowMode::CENTER, 0, 0);
    auto second = c.update(1u << 6, FollowMode::CENTER, 0, 0);
    assert(second.rightSpeed != first.rightSpeed);
    for (int i = 0; i < 1000; ++i) c.update(1u << 13, FollowMode::CENTER, 0, 0);
    auto capped = c.update(1u << 6, FollowMode::CENTER, 0, 0);
    assert(capped.rightSpeed >= -100 && capped.rightSpeed <= 100);
    c.update(0, FollowMode::CENTER, 0, 0);
    auto reset = c.update(1u << 6, FollowMode::CENTER, 0, 0);
    assert(reset.rightSpeed == first.rightSpeed);
    PIDController terms(p);
    auto proportional = terms.update(1u << 13, FollowMode::CENTER, 1, 0);
    terms.reset();
    auto derivative = terms.update(1u << 13, FollowMode::CENTER, 2, 0);
    assert(proportional.rightSpeed == derivative.rightSpeed);
    PIDController d;
    auto a = d.update(1u << 13, FollowMode::CENTER, 99, 50);
    d.reset();
    auto b = d.update(1u << 13, FollowMode::CENTER, 0, 50);
    assert(a.rightSpeed == b.rightSpeed && a.leftSpeed == b.leftSpeed);
}
static void test_first_valid_update_uses_legacy_zero_last_error_for_derivative() {
    std::array<PIDProfile, 3> p{{{0, 0, 1}, {0, 0, 0}, {0, 0, 0}}};
    PIDController c(p);
    auto first = c.update(1u << 13, FollowMode::CENTER, 0, 0);
    assert(first.rightSpeed == 26 && first.leftSpeed == -26);
}
static void test_timed_updates() {
    std::array<PIDProfile, 3> p{{{1, 1, 0}, {0, 0, 0}, {0, 0, 0}}};
    PIDController c(p);
    auto first = c.update(1u << 13, FollowMode::CENTER, 0, 0, 100, 10);
    auto early = c.update(1u << 13, FollowMode::CENTER, 0, 0, 105, 10);
    assert(early.rightSpeed == first.rightSpeed && early.leftSpeed == first.leftSpeed);
    auto accepted = c.update(1u << 13, FollowMode::CENTER, 0, 0, 110, 10);
    assert(accepted.rightSpeed != early.rightSpeed || accepted.leftSpeed != early.leftSpeed);
    auto lost = c.update(0, FollowMode::CENTER, 0, 0, 111, 10);
    assert(lost.rightSpeed == accepted.rightSpeed && lost.leftSpeed == accepted.leftSpeed);
}

static void test_timed_rollover() {
    PIDController c;
    auto first = c.update(1u << 13, FollowMode::CENTER, 0, 50, 0xfffffff8u, 10);
    auto early = c.update(1u << 13, FollowMode::CENTER, 0, 50, 1u, 10);
    assert(early.rightSpeed == first.rightSpeed && early.leftSpeed == first.leftSpeed);
    auto accepted = c.update(1u << 13, FollowMode::CENTER, 0, 50, 2u, 10);
    assert(accepted.rightSpeed != early.rightSpeed || accepted.leftSpeed != early.leftSpeed);
}

static void test_mapping_and_clamps() {
    PIDController c;
    auto zero = c.update(0x3fff, FollowMode::CENTER, 0, 0);
    auto full = c.update(0x3fff, FollowMode::CENTER, 0, 100);
    assert(zero.rightSpeed == 0 && zero.leftSpeed == 0);
    assert(full.rightSpeed == 255 && full.leftSpeed == 255);
    std::array<PIDProfile, 3> huge{{{1000, 1000, 1000}, {0,0,0}, {0,0,0}}};
    PIDController h(huge);
    auto out = h.update(1u << 13, FollowMode::CENTER, 0, 100);
    assert(out.rightSpeed >= -255 && out.rightSpeed <= 255);
    assert(out.leftSpeed >= -255 && out.leftSpeed <= 255);
}
int main() { test_centered_and_lost(); test_weights_and_none(); test_pid_terms_profiles_and_reset(); test_first_valid_update_uses_legacy_zero_last_error_for_derivative(); test_mapping_and_clamps(); test_timed_updates(); test_timed_rollover(); }
