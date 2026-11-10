#include "storage.h"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

static uint32_t checksum(const std::vector<uint8_t>& bytes) {
    uint32_t hash = 2166136261u;
    for (uint8_t byte : bytes) { hash ^= byte; hash *= 16777619u; }
    return hash;
}

static void rewriteChecksum(std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> payload(bytes.begin(), bytes.begin() + 8);
    payload.insert(payload.end(), bytes.begin() + 12, bytes.end());
    const uint32_t value = checksum(payload);
    for (int i = 0; i < 4; ++i) bytes[8 + i] = static_cast<uint8_t>(value >> (i * 8));
}

static MemoryProgram sample(uint8_t n) {
    MemoryProgram memory{};
    initializeDefaultMemory(memory);
    memory.stopPlan = static_cast<uint16_t>(n + 20);
    memory.steps[0].action = Action::MOVE_FWD;
    memory.steps[149].rightSpeed = static_cast<int16_t>(n * 3);
    return memory;
}

static std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), {}};
}

static void writeFile(const std::string& path, const std::vector<uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    assert(output);
}

int main() {
    const char* storageRoot = "/tmp/line_foll_storage";
    const std::string file = std::string(storageRoot) + "/memory_0";
    std::filesystem::remove_all(storageRoot);
    assert(initializeStorage(storageRoot));

    MemoryProgram missing{};
    assert(loadMemory(0, missing));
    assert(missing.stopPlan == STEP_COUNT && missing.steps[149].action == Action::FINISH);

    const MemoryProgram m0 = sample(0);
    const MemoryProgram m15 = sample(15);
    assert(saveMemory(0, m0));
    assert(saveMemory(15, m15));
    assert(!saveMemory(16, m0));
    assert(!loadMemory(16, missing));

    MemoryProgram loaded{};
    assert(loadMemory(0, loaded));
    assert(loaded.stopPlan == m0.stopPlan && loaded.steps[149].rightSpeed == 0);
    assert(loadMemory(15, loaded));
    assert(loaded.stopPlan == m15.stopPlan && loaded.steps[149].rightSpeed == 45);

    MemoryProgram invalid = sample(9);
    invalid.stopPlan = STEP_COUNT + 1;
    assert(!saveMemory(0, invalid));

    // Unsupported v1 fixture rejected and replaced with defaults.
    assert(saveMemory(0, sample(1)));
    std::vector<uint8_t> v1 = readFile(file);
    v1[4] = 1; v1[5] = 0;
    rewriteChecksum(v1);
    writeFile(file, v1);
    assert(loadMemory(0, loaded));
    assert(loaded.stopPlan == STEP_COUNT);
    assert(loaded.steps[0].action == Action::FINISH);

    // Break caught: decoder's per-step byte guard understates serialized record length.
    assert(saveMemory(0, sample(1)));
    std::vector<uint8_t> truncated = readFile(file);
    assert(truncated.size() > 12);
    truncated.resize(truncated.size() - 1);
    rewriteChecksum(truncated);
    writeFile(file, truncated);
    assert(loadMemory(0, loaded));
    assert(loaded.stopPlan == STEP_COUNT);
    assert(loaded.steps[0].action == Action::FINISH);

    // Break caught: save accepts jump target outside valid memory or step range.
    MemoryProgram jump = sample(2);
    jump.steps[0].jumpEnabled = true;
    jump.steps[0].jumpTargetMemory = MEMORY_COUNT;
    assert(!saveMemory(0, jump));
    jump.steps[0].jumpTargetMemory = 0;
    jump.steps[0].jumpTargetStep = STEP_COUNT;
    assert(!saveMemory(0, jump));

    // Break caught: save accepts sensor masks with bits outside 14 sensors.
    MemoryProgram mask = sample(3);
    mask.steps[0].sensorMaskA = static_cast<SensorMask>(0x4000);
    assert(!saveMemory(0, mask));
    mask.steps[0].sensorMaskA = SensorMask::NONE;
    mask.steps[0].sensorMaskB = static_cast<SensorMask>(0x8000);
    assert(!saveMemory(0, mask));

    // Break caught: save accepts values outside serialized enum domains.
    MemoryProgram enums = sample(4);
    enums.steps[0].action = static_cast<Action>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].action = Action::MOVE_FWD;
    enums.steps[0].sensor = static_cast<Sensor>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].sensor = Sensor::IGN;
    enums.steps[0].sensorTriggerMode = static_cast<SensorTriggerMode>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].sensorTriggerMode = SensorTriggerMode::NONE;
    enums.steps[0].counterMode = static_cast<CounterMode>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].counterMode = CounterMode::NONE;
    enums.steps[0].counterAMode = static_cast<CounterMode>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].counterAMode = CounterMode::NONE;
    enums.steps[0].counterBMode = static_cast<CounterMode>(255);
    assert(!saveMemory(0, enums));
    enums.steps[0].counterBMode = CounterMode::NONE;
    enums.steps[0].followMode = static_cast<FollowMode>(255);
    assert(!saveMemory(0, enums));

    // Ki outside documented range rejected.
    MemoryProgram invalidKi = sample(6);
    invalidKi.steps[0].pidKi = 256;
    assert(!saveMemory(0, invalidKi));
    invalidKi.steps[0].pidKi = -256;
    assert(!saveMemory(0, invalidKi));

    // Every serialized StepPlan field survives round trip.
    MemoryProgram roundTrip = sample(5);
    StepPlan& source = roundTrip.steps[0];
    source.action = Action::PICK;
    source.sensor = Sensor::S5;
    source.sensorTriggerMode = SensorTriggerMode::EQ;
    source.sensorMaskA = static_cast<SensorMask>(0x00ff);
    source.sensorMaskB = static_cast<SensorMask>(0x3f00);
    source.rightSpeed = -123;
    source.leftSpeed = 456;
    source.counterMode = CounterMode::TIMER;
    source.counter = 99999;
    source.valueA = -500;
    source.counterA = 12345;
    source.counterAMode = CounterMode::ENCODERL;
    source.valueB = 600;
    source.counterB = -7890;
    source.counterBMode = CounterMode::ENCODERR;
    source.pidProfile = 4;
    source.pidKi = -17;
    source.followMode = FollowMode::LEFT;
    source.jumpEnabled = true;
    source.jumpTargetMemory = 3;
    source.jumpTargetStep = 42;
    assert(saveMemory(0, roundTrip));
    assert(loadMemory(0, loaded));
    assert(loaded.stopPlan == roundTrip.stopPlan);
    const StepPlan& restored = loaded.steps[0];
    assert(restored.action == source.action);
    assert(restored.sensor == source.sensor);
    assert(restored.sensorTriggerMode == source.sensorTriggerMode);
    assert(restored.sensorMaskA == source.sensorMaskA);
    assert(restored.sensorMaskB == source.sensorMaskB);
    assert(restored.rightSpeed == source.rightSpeed);
    assert(restored.leftSpeed == source.leftSpeed);
    assert(restored.counterMode == source.counterMode);
    assert(restored.counter == source.counter);
    assert(restored.valueA == source.valueA);
    assert(restored.counterA == source.counterA);
    assert(restored.counterAMode == source.counterAMode);
    assert(restored.valueB == source.valueB);
    assert(restored.counterB == source.counterB);
    assert(restored.counterBMode == source.counterBMode);
    assert(restored.pidProfile == source.pidProfile);
    assert(restored.pidKi == source.pidKi);
    assert(restored.followMode == source.followMode);
    assert(restored.jumpEnabled == source.jumpEnabled);
    assert(restored.jumpTargetMemory == source.jumpTargetMemory);
    assert(restored.jumpTargetStep == source.jumpTargetStep);

    std::cout << "All storage tests passed!\n";
}
