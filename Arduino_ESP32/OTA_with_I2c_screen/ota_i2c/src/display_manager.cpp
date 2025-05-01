#include "display_manager.h"
#include <Wire.h>

DisplayManager::DisplayManager(int width, int height, int resetPin) 
    : _display(width, height, &Wire, resetPin) {}

bool DisplayManager::begin(uint8_t i2cAddress) {
    if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        return false;
    }
    _setupDisplay();
    return true;
}

void DisplayManager::_setupDisplay() {
    _display.clearDisplay();
    _display.display();
}

void DisplayManager::clear() {
    _display.clearDisplay();
}

void DisplayManager::display() {
    _display.display();
}

void DisplayManager::_setupTextDisplay() {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
}

void DisplayManager::displayText(const char* text, int line) {
    _setupTextDisplay();
    _display.setCursor(0, line * 10);
    _display.println(text);
    _display.display();
}

void DisplayManager::displayLines(const std::vector<String>& lines) {
    _setupTextDisplay();
    
    for (size_t i = 0; i < lines.size(); i++) {
        _display.setCursor(0, i * 10);
        _display.println(lines[i].c_str());
    }
    
    _display.display();
}

void DisplayManager::displayMemoryInfo() {
    std::vector<String> lines;
    char memInfo[64];
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freePsram = ESP.getFreePsram();
    uint32_t totalPsram = ESP.getPsramSize();
    uint32_t freeSketchSpace = ESP.getFreeSketchSpace();
    uint32_t sketchSize = ESP.getSketchSize();
    
    sprintf(memInfo, "Heap: %u/%u", freeHeap, totalHeap);
    lines.push_back(String(memInfo));
    
    if (totalPsram > 0) {
        sprintf(memInfo, "PSRAM: %u/%u", freePsram, totalPsram);
        lines.push_back(String(memInfo));
    }
    
    sprintf(memInfo, "Flash: %u/%u", freeSketchSpace, sketchSize);
    lines.push_back(String(memInfo));
    
    displayLines(lines);
} 