# Memory StepPlan Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace separate Plan/Step hierarchy with 16 LittleFS-backed memory programs, each containing 150 StepPlans, and implement safe defaults, stop behavior, and coalesced auto-save.

**Architecture:** Keep only one `MemoryProgram` active in RAM. Store each memory in one independent file, with versioned serialization and atomic replace. Separate unit-testable model/default/stop logic from the LittleFS adapter and menu editing integration.

**Tech Stack:** ESP32-C3 Arduino C++, LittleFS, host-side C++ tests.

**Spec:** `docs/superpowers/specs/2026-11-10-memory-stepplan-design.md`

## Global Constraints

- `MEMORY_COUNT = 16`, `STEP_COUNT = 150`, `CHECKPOINT_COUNT = 30`, `CALIBRATION_PROFILE_COUNT = 5`.
- Every memory has `StepPlan 0..149`; no separate Plan level.
- Store one memory per LittleFS file `/memory_00.bin` through `/memory_15.bin`.
- RAM holds only one active `MemoryProgram`, never all 16 programs.
- `stopPlan` range is `0..150`; `150` disables stop boundary.
- Stop boundary is checked before executing a step: `stopPlan = 20` allows steps `0..19`.
- `Action::FINISH` stops robot when that step executes, regardless of `stopPlan`.
- All steps default to `FINISH`, sensor `IGN`, speeds zero, counter modes `NONE`, numeric parameters zero, PID profile zero, follow mode `NONE`, jump disabled and targets zero.
- Auto-save coalesces edits for 300 ms; leaving editor or changing memory flushes pending save immediately.
- Preserve ESP32-C3 pin mapping and `uiMain.h` assets/layout.

## Review Focus

- Corrupt, truncated, or unsupported-version file must not replace valid memory; test validation and failed load behavior in Task 2.
- `stopPlan` boundaries 0, 20, 149, and 150 must follow exact before-step semantics; test in Task 3.
- `FINISH` at step 0 and after ordinary steps must halt regardless of `stopPlan`; test in Task 3.
- Repeated edits inside 300 ms must cause one save, and edit at deadline must persist; test in Task 4.
- Switching memory must flush prior dirty memory and never mix memory records; test in Task 4.

---

## File Structure

- `config.h`: finalized capacities and memory-file constants.
- `models.h`: enums and POD model types for `StepPlan` and `MemoryProgram`.
- `storage.h/.cpp`: LittleFS-backed load/save, versioned serialization, atomic replacement, and explicit memory index validation.
- `planner.h/.cpp`: unit-testable next-step/stop decision logic and run-state handling.
- `menu.h/.cpp`: memory/step editing state and memory-switch flush hooks.
- `test_models.cpp`, `test_storage.cpp`, `test_planner.cpp`, `test_autosave.cpp`: host regression coverage.

## Task 1: Define final StepPlan model and defaults

**Files:** Modify `models.h`, `config.h`; create `test_models.cpp`.

**Interfaces:** Produces `struct StepPlan`, `struct MemoryProgram { StepPlan steps[STEP_COUNT]; uint16_t stopPlan; }`, `initializeDefaultStep(StepPlan&)`, and `initializeDefaultMemory(MemoryProgram&)`.

- [ ] Write tests for all default field values, 150 steps, `stopPlan == 150`, and compile-time capacities.
- [ ] Run tests and confirm failure because new model/default functions are absent.
- [ ] Implement enums `Action::FINISH`, `Sensor::IGN`, `CounterMode::NONE`, `FollowMode::NONE` and model fields matching design spec.
- [ ] Run `g++ -std=c++17 -Wall -Wextra -Werror test_models.cpp -o /tmp/test_models && /tmp/test_models`; expect PASS.

## Task 2: Persist one memory program per LittleFS file

**Files:** Replace relevant `storage.h/.cpp` API; modify `test_storage.cpp`.

**Interfaces:** `bool initializeStorage(); bool loadMemory(uint8_t memoryIndex, MemoryProgram& out); bool saveMemory(uint8_t memoryIndex, const MemoryProgram& memory); bool backupData(const char* destination);`

- [ ] Test save/load isolation for M0 and M15, invalid index 16, exact 150 steps, stopPlan 150, and default initialization for missing file; confirm RED first.
- [ ] Test restart reload, checksum/version rejection, truncated-file rejection, and preservation of prior valid file after failed atomic write.
- [ ] Implement host-test backend and Arduino `LittleFS` backend behind same API; serialize fields explicitly rather than raw struct bytes, include magic/version/checksum, write temp then flush and rename.
- [ ] Run storage and model tests with `-Wall -Wextra -Werror`; expect PASS.

## Task 3: Implement stop and FINISH execution rules

**Files:** Create `planner.h/.cpp`; create `test_planner.cpp`.

**Interfaces:** `bool shouldStopBeforeStep(const MemoryProgram&, uint16_t currentStep); bool shouldStopAfterStep(const StepPlan&); uint16_t nextStepIndex(uint16_t currentStep);`

- [ ] Test stopPlan 0 prevents step 0, 20 permits 0..19 but prevents 20, 149 prevents 149, and 150 never stops by boundary; confirm RED.
- [ ] Test `FINISH` stops at step 0 and later step; ordinary action does not stop by itself.
- [ ] Implement planner decisions without hardware dependencies.
- [ ] Run planner tests with `-Wall -Wextra -Werror`; expect PASS.

## Task 4: Coalesced automatic save and memory switching

**Files:** Create/modify menu editor state in `menu.h/.cpp`; create `test_autosave.cpp`.

**Interfaces:** `markMemoryDirty(uint32_t nowMs); bool autosaveTick(uint32_t nowMs); bool flushPendingSave(); bool selectMemory(uint8_t memoryIndex);` Use injected storage callback or narrow storage interface for deterministic host tests.

- [ ] Test edits at 0, 100, and 250 ms result in no save before 550 ms and one save at/after 550 ms; confirm RED.
- [ ] Test a new edit resets deadline to 300 ms after latest edit.
- [ ] Test editor exit and memory switch flush immediately, and prior memory data remains isolated.
- [ ] Implement timer coalescing with wrap-safe unsigned `millis()` subtraction; do not sleep/block in menu loop.
- [ ] Run autosave, storage, planner, and model tests with `-Wall -Wextra -Werror`; expect PASS.

## Task 5: Integrate menu editing and robot run selection

**Files:** Modify `menu.h/.cpp`, create `test_menu_stepplan.cpp`.

**Interfaces:** Menu state tracks selected memory `0..15`, selected step `0..149`, selected parameter, and current editor/run screen. Existing navigation constraints remain: main2 ENTER opens planning/editor; main3 ENTER starts selected checkpoint/memory; main5 BACK opens Config.

- [ ] Test memory and StepPlan cursor bounds, main2 editor entry, back/exit flush, and main3 run request carrying selected memory/checkpoint; confirm RED.
- [ ] Implement menu integration using APIs from Tasks 1–4; retain existing `uiMain.h` main screen rendering contract.
- [ ] Test each action parameter edit updates active StepPlan and schedules save; test stopPlan edit updates memory and schedules save.
- [ ] Run complete host suite; expect PASS.

## Task 6: Verification and ESP32-C3 build readiness

**Files:** Modify existing build configuration only if present; add a concise test/build note if needed.

- [ ] Run every host test with `-Wall -Wextra -Werror`.
- [ ] Compile firmware with configured ESP32-C3 Arduino toolchain and LittleFS enabled; report exact command and result.
- [ ] Confirm no legacy `MAXPLAN` hierarchy or `PBxx`/`PAxx`/`PCxx` pin names remain in new modules.
- [ ] Document any hardware/toolchain verification not possible in current environment; do not claim board-level behavior without hardware test.
