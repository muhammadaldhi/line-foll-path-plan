# Auxiliary Hardware Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add host-testable servo, encoder, and buzzer modules that remain functional in software while physical pins stay `-1`.

**Architecture:** Three independent modules expose state and command APIs. Hardware callbacks are injected on host; Arduino-specific output is guarded. Pin `-1` skips physical I/O but does not invalidate software state.

**Tech Stack:** C++17 host tests, ESP32-C3 Arduino compatibility, existing `config.h`.

**Spec:** `docs/superpowers/specs/2026-11-10-auxiliary-hardware-design.md`

## Global Constraints

- Servo angle is clamped to `0..180`.
- Encoder counts are signed.
- Invalid quadrature transitions are ignored.
- Buzzer pin `-1` records last event but performs no hardware I/O.
- No planner behavior changes.
- No direct Arduino dependency in host execution paths.

## Review Focus

- Servo out-of-range angle must clamp, not wrap.
- Encoder invalid transition must not change count.
- Encoder reverse sequence must decrement count.
- Buzzer pin `-1` must retain event state without callback.
- All modules must compile with `-Wall -Wextra -Werror` without Arduino.

---

### Task 1: Servo controller

**Files:**
- Create: `servo.h`
- Create: `servo.cpp`
- Create: `test_servo.cpp`

**Interfaces:**
```cpp
using ServoWriteCallback = std::function<void(int8_t, uint8_t)>;
class ServoController {
public:
    ServoController(ServoWriteCallback write, int8_t leftPin = SERVO_L_PIN,
                    int8_t rightPin = SERVO_R_PIN);
    void setLeft(int16_t angle);
    void setRight(int16_t angle);
    void stop();
    uint8_t leftAngle() const;
    uint8_t rightAngle() const;
};
```

- [ ] Write failing tests for clamping, state, callback pins, and `-1` pin skip.
- [ ] Run RED.
- [ ] Implement minimal servo controller and Arduino guard.
- [ ] Run `g++ -std=c++17 -Wall -Wextra -Werror test_servo.cpp servo.cpp -o /tmp/test_servo && /tmp/test_servo`.

### Task 2: Quadrature encoder controller

**Files:**
- Create: `encoder.h`
- Create: `encoder.cpp`
- Create: `test_encoder.cpp`

**Interfaces:**
```cpp
class EncoderController {
public:
    void updateLeft(bool a, bool b);
    void updateRight(bool a, bool b);
    int32_t leftCount() const;
    int32_t rightCount() const;
    void reset();
};
```

- [ ] Write failing tests for forward sequence, reverse sequence, invalid transitions, reset, and signed counts.
- [ ] Run RED.
- [ ] Implement 2-bit quadrature state table; ignore invalid two-bit jumps.
- [ ] Add Arduino interrupt setup only when encoder pins are not `-1`.
- [ ] Run `g++ -std=c++17 -Wall -Wextra -Werror test_encoder.cpp encoder.cpp -o /tmp/test_encoder && /tmp/test_encoder`.

### Task 3: Buzzer controller

**Files:**
- Create: `buzzer.h`
- Create: `buzzer.cpp`
- Create: `test_buzzer.cpp`

**Interfaces:**
```cpp
struct BuzzerEvent { uint16_t frequency; uint16_t durationMs; bool active; };
using BuzzerCallback = std::function<void(int8_t, uint16_t, uint16_t)>;
class BuzzerController {
public:
    BuzzerController(BuzzerCallback callback, int8_t pin = BUZZER_PIN);
    void beep(uint16_t frequency, uint16_t durationMs);
    void stop();
    BuzzerEvent lastEvent() const;
};
```

- [ ] Write failing tests for event recording, stop, callback, and `-1` pin skip.
- [ ] Run RED.
- [ ] Implement minimal controller and Arduino tone/noTone guard.
- [ ] Run `g++ -std=c++17 -Wall -Wextra -Werror test_buzzer.cpp buzzer.cpp -o /tmp/test_buzzer && /tmp/test_buzzer`.

### Task 4: Full regression verification

- [ ] Run all auxiliary tests.
- [ ] Run existing model, planner, storage, menu, display, sensor, button, motor, and PID tests.
- [ ] Confirm no planner or storage behavior changed.
