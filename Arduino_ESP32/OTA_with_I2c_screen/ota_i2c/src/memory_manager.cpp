#include "memory_manager.h"

MemoryManager::MemoryStatus MemoryManager::getStatus() 
{
    MemoryStatus status;
    status.freeHeap = ESP.getFreeHeap();
    status.totalHeap = ESP.getHeapSize();
    status.freePsram = ESP.getFreePsram();
    status.totalPsram = ESP.getPsramSize();
    status.freeSketchSpace = ESP.getFreeSketchSpace();
    status.sketchSize = ESP.getSketchSize();
    return status;
}

String MemoryManager::getStatusJson() 
{
    MemoryStatus status = getStatus();
    char response[256];
    sprintf(response, 
            "{\"heap\":{\"free\":%u,\"total\":%u},\"psram\":{\"free\":%u,\"total\":%u},\"flash\":{\"free\":%u,\"total\":%u}}",
            status.freeHeap, status.totalHeap, 
            status.freePsram, status.totalPsram, 
            status.freeSketchSpace, status.sketchSize);
    return String(response);
}

std::vector<String> MemoryManager::getStatusLines() 
{
    MemoryStatus status = getStatus();
    std::vector<String> lines;
    char memInfo[64];
    
    sprintf(memInfo, "Heap: %u/%u", status.freeHeap, status.totalHeap);
    lines.push_back(String(memInfo));
    
    if (status.totalPsram > 0) 
    {
        sprintf(memInfo, "PSRAM: %u/%u", status.freePsram, status.totalPsram);
        lines.push_back(String(memInfo));
    }
    
    sprintf(memInfo, "Flash: %u/%u", status.freeSketchSpace, status.sketchSize);
    lines.push_back(String(memInfo));
    
    return lines;
} 