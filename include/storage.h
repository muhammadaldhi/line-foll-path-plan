#ifndef STORAGE_H
#define STORAGE_H

#include "models.h"

bool initializeStorage(const char* rootPath = "storage");
bool loadMemory(uint8_t memoryIndex, MemoryProgram& out);
bool saveMemory(uint8_t memoryIndex, const MemoryProgram& memory);
bool backupData(const char* destination);

#endif
