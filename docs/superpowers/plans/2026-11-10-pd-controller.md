# PD Controller Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port source legacy `flc`, `flr`, and `fll` into host-testable `PIDController` behavior without direct hardware calls.

**Architecture:** `PIDController` receives a 14-bit active sensor mask and produces signed right/left motor speeds. It implements full PID (Proportional, Integral, Derivative) with anti-windup. It preserves legacy weighted sensor positions, `kp/kd` profiles, follow modes, speed mapping, timing, and output clamping. `MotorController` remains the only hardware output layer.

**Tech Stack:** C++17 host tests, ESP32-C3 Arduino compatibility, existing `models.h`, `sensors.h`, `motors.h`.

**Spec:** `docs/superpowers/specs/2026-11-10-memory-stepplan-design.md`

## Global Constraints

- Full PID behavior.
- Center weights: `{-26,-22,-18,-14,-10,-5,-1,1,5,10,14,18,22,26}`.
- Right-follow weights: `{-46,-42,-38,-34,-30,-26,-22,-18,-14,-10,-5,-1,1,5}`.
- Left-follow weights: `{-5,-1,1,5,10,14,18,22,26,30,34,38,42,46}`.
- Default profiles: `kp={10,13,20}`, `kd={20,25,50}`, `ki={0,0,0}`.
- Base speed input uses legacy `0..100` scale and maps to `0..255`.
- Output speed clamps to `-255..255`.
- Anti-windup: limit integral contribution to +/- 100.
- Reset integral on `lineLost`, `reset()`, or `step()` change.
- Controller does not call GPIO, Arduino motor APIs, or sensor hardware APIs.

## Review Focus

- Integral windup: ensure `moveVal` doesn't runaway; test in Task 1.
- No active sensor bits: return `lineLost=true`, zero motor output, and reset integral; test in Task 1.
- Zero-crossing: optional reset or dampening of integral when error changes sign; test in Task 1.
- Invalid PID profile index: use profile 0; test in Task 1.
- `Ki` storage: ensure `Ki` persists and is editable; test in Task 4.

---

### Task 1: Implement full PID calculation

**Files:**
- Create: `pid.h`
- Create: `pid.cpp`
- Create: `test_pid.cpp`

**Interfaces:**
- Consumes: `FollowMode` from `models.h` and `SENSOR_COUNT` from `config.h`.
- Produces:
  ```cpp
  struct PIDProfile { int16_t kp; int16_t ki; int16_t kd; };
  struct MotorCommand { int16_t rightSpeed; int16_t leftSpeed; bool lineLost; };
  class PIDController {
  public:
      explicit PIDController(std::array<PIDProfile, 3> profiles = defaultProfiles());
      MotorCommand update(uint16_t activeMask, FollowMode mode,
                          uint8_t profile, uint8_t baseSpeed);
      void reset();
  };
  ```

- [ ] **Step 1: Write failing tests**
  - PID correction with non-zero `ki` accumulates error over time.
  - Anti-windup prevents infinite integral growth.
  - Integral resets on `lineLost`.
  - Centered mask produces equal left/right speeds.
  - Profile values (P, I, D) affect correction.
  - Base speed maps `0..100` to `0..255`.
  - Output clamps to `-255..255`.
  - Empty mask returns `lineLost=true` and zero output.

- [ ] **Step 2: Run RED**
  ```bash
  g++ -std=c++17 -Wall -Wextra -Werror test_pid.cpp pid.cpp -o /tmp/test_pid
  ```

- [ ] **Step 3: Implement PID controller**
  - Calculate error from weighted average.
  - `integral += error`.
  - Clamp `integral` (anti-windup).
  - `moveVal = error * kp + integral * ki + (error - lastError) * kd`.
  - Set speeds and clamp.

- [ ] **Step 4: Run GREEN**

### Task 2: Add legacy update timing

**Files:**
- Modify: `pid.h`
- Modify: `pid.cpp`
- Modify: `test_pid.cpp`

- [ ] **Step 1: Write failing timing tests**
  - Update before interval returns previous command.
  - Update after interval advances PID state.

- [ ] **Step 2: Run RED**

- [ ] **Step 3: Implement timing**
  - Use `nowMs - lastUpdateMs >= updateIntervalMs`.

- [ ] **Step 4: Run GREEN**

### Task 3: Update Models, Storage, and Menu for Ki

**Files:**
- Modify: `models.h`
- Modify: `storage.cpp`
- Modify: `menu.h`
- Modify: `menu.cpp`
- Modify: `test_storage.cpp`
- Modify: `test_menu_stepplan.cpp`

- [ ] **Step 1: Write failing tests**
  - `PIDProfile` contains `ki`.
  - `storage` saves/loads `ki` (version bump to 2 required for layout change).
  - `menu` can edit `ki` parameter.

- [ ] **Step 2: Run RED**

- [ ] **Step 3: Implement Ki support**
  - Update `PIDProfile` struct.
  - Bump `STORAGE_VERSION` to 2.
  - Update `putStep/getStep` for `ki`.
  - Add `KI` to `StepPlanParameter` enum.
  - Add menu logic to handle `KI` value change.

- [ ] **Step 4: Run GREEN**

### Task 4: Integrate PID with Motor Driver

**Files:**
- Modify: `pid.h`
- Create: `test_pid_motors.cpp`

- [ ] **Step 1: Write integration test**
- [ ] **Step 2: Run RED**
- [ ] **Step 3: Implement integration**
- [ ] **Step 4: Run GREEN and full suite**

