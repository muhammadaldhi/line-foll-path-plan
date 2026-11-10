#include <cassert>
#include <cstdint>
#include <vector>
#include "menu.h"
#include "models.h"

struct SaveCall { uint8_t index; MemoryProgram memory; };

int main() {
    std::vector<SaveCall> saves;
    bool saveAllowed = true;
    MemoryProgram memories[2];
    initializeDefaultMemory(memories[0]);
    initializeDefaultMemory(memories[1]);
    memories[0].steps[0].rightSpeed = 11;
    memories[1].steps[0].rightSpeed = 22;

    MemoryEditorStorage storage;
    storage.load = [&](uint8_t index, MemoryProgram& out) {
        if (index >= 2) return false;
        out = memories[index];
        return true;
    };
    storage.save = [&](uint8_t index, const MemoryProgram& memory) {
        if (!saveAllowed) return false;
        saves.push_back({index, memory});
        memories[index] = memory;
        return true;
    };
    configureMemoryEditor(storage, 0);

    assert(activeMemory().steps[0].rightSpeed == 11);
    activeMemory().steps[1].rightSpeed = 7;
    markMemoryDirty(0);
    activeMemory().steps[1].rightSpeed = 8;
    markMemoryDirty(100);
    activeMemory().steps[1].rightSpeed = 9;
    markMemoryDirty(250);
    assert(!autosaveTick(549));
    assert(saves.empty());
    assert(autosaveTick(550));
    assert(saves.size() == 1);
    assert(saves[0].memory.steps[1].rightSpeed == 9);
    assert(!autosaveTick(551));

    activeMemory().steps[1].rightSpeed = 12;
    markMemoryDirty(600);
    saveAllowed = false;
    assert(!autosaveTick(900));
    saveAllowed = true;
    assert(autosaveTick(901));

    activeMemory().steps[2].rightSpeed = 10;
    markMemoryDirty(1000);
    assert(!autosaveTick(1200));
    assert(autosaveTick(1300));
    assert(saves.size() == 3);

    activeMemory().steps[3].rightSpeed = 33;
    markMemoryDirty(2000);
    assert(flushPendingSave());
    assert(saves.size() == 4);
    assert(!flushPendingSave());

    activeMemory().steps[4].rightSpeed = 44;
    markMemoryDirty(3000);
    assert(selectMemory(1));
    assert(saves.size() == 5);
    assert(saves.back().index == 0);
    assert(activeMemory().steps[0].rightSpeed == 22);
    assert(!selectMemory(16));
    assert(activeMemory().steps[0].rightSpeed == 22);

    activeMemory().steps[5].rightSpeed = 66;
    markMemoryDirty(4000);
    saveAllowed = false;
    assert(!selectMemory(0));
    assert(activeMemory().steps[5].rightSpeed == 66);
    saveAllowed = true;
    assert(selectMemory(0));
    assert(activeMemory().steps[0].rightSpeed == 11);

    configureMemoryEditor(storage, 0);
    activeMemory().steps[5].rightSpeed = 55;
    markMemoryDirty(UINT32_MAX - 100);
    assert(!autosaveTick(100));
    assert(autosaveTick(200));
    return 0;
}
