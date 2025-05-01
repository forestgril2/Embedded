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

void DisplayManager::displayText(const char* text, int line) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, line * 10);
    _display.setTextColor(SSD1306_WHITE);
    _display.println(text);
    _display.display();
}

void DisplayManager::displayLines(const char* line1, const char* line2, const char* line3) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    _display.println(line1);
    _display.setCursor(0, 10);
    _display.println(line2);
    _display.setCursor(0, 20);
    _display.println(line3);
    _display.display();
}

void DisplayManager::displayMemoryInfo() {
    char memInfo[64];
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freePsram = ESP.getFreePsram();
    uint32_t totalPsram = ESP.getPsramSize();
    uint32_t freeSketchSpace = ESP.getFreeSketchSpace();
    uint32_t sketchSize = ESP.getSketchSize();
    
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    
    sprintf(memInfo, "Heap: %u/%u", freeHeap, totalHeap);
    _display.println(memInfo);
    
    if (totalPsram > 0) {
        sprintf(memInfo, "PSRAM: %u/%u", freePsram, totalPsram);
        _display.println(memInfo);
    }
    
    sprintf(memInfo, "Flash: %u/%u", freeSketchSpace, sketchSize);
    _display.println(memInfo);
    
    _display.display();
} 