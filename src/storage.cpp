#include "storage.h"
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifdef ARDUINO
#include <LittleFS.h>
#endif

namespace {
constexpr uint32_t MAGIC = 0x4C464D50u;
// Storage files use magic, version, and checksum. Reject unsupported versions;
// increment VERSION whenever serialized layout changes incompatibly.
constexpr uint16_t VERSION = 2;
constexpr size_t SERIALIZED_STEP_SIZE = 42;
std::string root;

bool validStep(const StepPlan& s) {
    const auto action = static_cast<uint8_t>(s.action);
    const auto sensor = static_cast<uint8_t>(s.sensor);
    const auto trigger = static_cast<uint8_t>(s.sensorTriggerMode);
    const auto counter = static_cast<uint8_t>(s.counterMode);
    const auto counterA = static_cast<uint8_t>(s.counterAMode);
    const auto counterB = static_cast<uint8_t>(s.counterBMode);
    const auto follow = static_cast<uint8_t>(s.followMode);
    return action <= static_cast<uint8_t>(Action::FINISH) &&
           sensor < SENSOR_COUNT && trigger <= static_cast<uint8_t>(SensorTriggerMode::XOR) &&
           counter <= static_cast<uint8_t>(CounterMode::ENCODERL) &&
           counterA <= static_cast<uint8_t>(CounterMode::ENCODERL) &&
           counterB <= static_cast<uint8_t>(CounterMode::ENCODERL) &&
           follow <= static_cast<uint8_t>(FollowMode::RIGHT) &&
           s.pidKi >= -255 && s.pidKi <= 255 &&
           to_underlying(s.sensorMaskA) <= to_underlying(SensorMask::ALL) &&
           to_underlying(s.sensorMaskB) <= to_underlying(SensorMask::ALL) &&
           (!s.jumpEnabled || (s.jumpTargetMemory < MEMORY_COUNT && s.jumpTargetStep < STEP_COUNT));
}

uint32_t checksum(const std::vector<uint8_t>& b) {
    uint32_t h = 2166136261u;
    for (uint8_t v : b) { h ^= v; h *= 16777619u; }
    return h;
}
void put16(std::vector<uint8_t>& b, uint16_t v) { b.push_back(v); b.push_back(v >> 8); }
void put32(std::vector<uint8_t>& b, uint32_t v) { for (int i=0;i<4;++i) b.push_back(v >> (i*8)); }
uint16_t get16(const std::vector<uint8_t>& b, size_t& p) { uint16_t lo=b[p++]; uint16_t hi=b[p++]; return lo | (hi << 8); }
uint32_t get32(const std::vector<uint8_t>& b, size_t& p) { uint32_t v=0; for(int i=0;i<4;++i)v|=uint32_t(b[p++])<<(i*8); return v; }
void putStep(std::vector<uint8_t>& b, const StepPlan& s) {
    b.push_back(static_cast<uint8_t>(s.action)); b.push_back(static_cast<uint8_t>(s.sensor));
    b.push_back(static_cast<uint8_t>(s.sensorTriggerMode));
    put16(b, static_cast<uint16_t>(s.sensorMaskA)); put16(b, static_cast<uint16_t>(s.sensorMaskB));
    put16(b, static_cast<uint16_t>(s.rightSpeed)); put16(b, static_cast<uint16_t>(s.leftSpeed));
    b.push_back(static_cast<uint8_t>(s.counterMode)); put32(b,s.counter); put32(b,s.valueA); put32(b,s.counterA);
    b.push_back(static_cast<uint8_t>(s.counterAMode)); put32(b,s.valueB); put32(b,s.counterB); b.push_back(static_cast<uint8_t>(s.counterBMode));
    b.push_back(s.pidProfile); put16(b, static_cast<uint16_t>(s.pidKi)); b.push_back(static_cast<uint8_t>(s.followMode)); b.push_back(s.jumpEnabled ? 1 : 0); b.push_back(s.jumpTargetMemory); put16(b,s.jumpTargetStep);
}
bool getStep(const std::vector<uint8_t>& b, size_t& p, StepPlan& s) {
    if (p + SERIALIZED_STEP_SIZE > b.size()) return false;
    s.action = Action(b[p++]);
    s.sensor = Sensor(b[p++]);
    s.sensorTriggerMode = SensorTriggerMode(b[p++]);
    s.sensorMaskA = SensorMask(get16(b, p));
    s.sensorMaskB = SensorMask(get16(b, p));
    s.rightSpeed = int16_t(get16(b, p));
    s.leftSpeed = int16_t(get16(b, p));
    s.counterMode = CounterMode(b[p++]);
    s.counter = get32(b, p);
    s.valueA = int32_t(get32(b, p));
    s.counterA = int32_t(get32(b, p));
    s.counterAMode = CounterMode(b[p++]);
    s.valueB = int32_t(get32(b, p));
    s.counterB = int32_t(get32(b, p));
    s.counterBMode = CounterMode(b[p++]);
    s.pidProfile = b[p++];
    s.pidKi = int16_t(get16(b, p));
    s.followMode = FollowMode(b[p++]);
    s.jumpEnabled = b[p++] != 0;
    s.jumpTargetMemory = b[p++];
    s.jumpTargetStep = get16(b, p);
    return true;
}
std::string path(uint8_t i) { char n[32]; std::snprintf(n,sizeof n,"%s/memory_%02u.bin",root.c_str(),unsigned(i)); return n; }
bool decode(const std::vector<uint8_t>& b, MemoryProgram& out) {
    if (b.size() < 12) return false;
    size_t p = 0;
    uint32_t magic = get32(b, p);
    uint16_t ver = get16(b, p);
    uint16_t stop = get16(b, p);
    uint32_t saved = get32(b, p);
    if (magic != MAGIC || ver != VERSION || stop > STEP_COUNT) return false;
    std::vector<uint8_t> payload(b.begin(),b.begin()+8); payload.insert(payload.end(),b.begin()+12,b.end()); if(saved!=checksum(payload)) return false;
    MemoryProgram m{};
    m.stopPlan = stop;
    for (auto& s : m.steps) {
        if (!getStep(b, p, s) || !validStep(s)) return false;
    }
    if (p != b.size()) return false;
    out = m;
    return true;
}
}

bool initializeStorage(const char* rootPath) {
    if (!rootPath || !*rootPath) return false;
    root = rootPath;
#ifdef ARDUINO
    return LittleFS.begin(true);
#else
    std::error_code ec; std::filesystem::create_directories(root,ec); return !ec;
#endif
}
bool saveMemory(uint8_t i,const MemoryProgram& m) {
    if (root.empty() || i >= MEMORY_COUNT || m.stopPlan > STEP_COUNT) return false;
    for (const auto& s : m.steps) {
        if (!validStep(s)) return false;
    }
    std::vector<uint8_t> b;
    put32(b, MAGIC); put16(b, VERSION); put16(b, m.stopPlan); put32(b, 0);
    for (const auto& s : m.steps) putStep(b, s);
    std::vector<uint8_t> payload(b.begin(), b.begin() + 8);
    payload.insert(payload.end(), b.begin() + 12, b.end());
    uint32_t c = checksum(payload);
    for (int n = 0; n < 4; ++n) b[8 + n] = uint8_t(c >> (n * 8));
    std::string dst = path(i), tmp = dst + ".tmp";
#ifdef ARDUINO
    File f=LittleFS.open(tmp.c_str(),"w"); if(!f)return false; f.write(b.data(),b.size()); f.flush(); f.close(); LittleFS.remove(dst.c_str()); return LittleFS.rename(tmp.c_str(),dst.c_str());
#else
    {std::ofstream f(tmp,std::ios::binary|std::ios::trunc); if(!f)return false; f.write((char*)b.data(),b.size()); f.flush(); if(!f)return false;} return std::rename(tmp.c_str(),dst.c_str())==0;
#endif
}
bool loadMemory(uint8_t i,MemoryProgram& out) {
    if (root.empty() || i >= MEMORY_COUNT) return false;
    std::ifstream f(path(i), std::ios::binary);
    if (!f) { initializeDefaultMemory(out); return true; }
    std::vector<uint8_t> b((std::istreambuf_iterator<char>(f)), {});
    if (!decode(b, out)) { initializeDefaultMemory(out); return true; }
    return true;
}
bool backupData(const char* destination) { if(root.empty()||!destination||!*destination)return false; std::ofstream out(destination,std::ios::binary|std::ios::trunc); if(!out)return false; for(uint8_t i=0;i<MEMORY_COUNT;++i){std::ifstream in(path(i),std::ios::binary); if(in)out<<in.rdbuf();} return bool(out); }
