#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>

class MemoryManager 
{
public:
    struct MemoryStatus {
        uint32_t freeHeap;
        uint32_t totalHeap;
        uint32_t freePsram;
        uint32_t totalPsram;
        uint32_t freeSketchSpace;
        uint32_t sketchSize;
    };

    static MemoryStatus getStatus();
    static String getStatusJson();
    static std::vector<String> getStatusLines();
};

#endif // MEMORY_MANAGER_H 