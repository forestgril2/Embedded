#include "display_manager.h"
#include <Wire.h>
#include "memory_manager.h"

DisplayManager::DisplayManager(int width, int height, int resetPin) : _display(width, height, &Wire, resetPin) {}

bool DisplayManager::init(uint8_t i2cAddress) 
{
    if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) 
    {
        _initialized = false;
        return false;
    }
    _setupDisplay();
    _initialized = true;
    return true;
}

void DisplayManager::_setupDisplay() { _display.clearDisplay(); _display.display(); }

void DisplayManager::clear() { _display.clearDisplay(); }

void DisplayManager::display() { _display.display(); }

void DisplayManager::_setupTextDisplay() 
{
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
}

void DisplayManager::displayText(const char* text, int line) 
{
    _setupTextDisplay();
    _display.setCursor(0, line * 10);
    _display.println(text);
    _display.display();
}

void DisplayManager::displayLines(const std::vector<String>& lines) 
{
    _setupTextDisplay();
    
    for (size_t i = 0; i < lines.size(); i++) 
    {
        _display.setCursor(0, i * 10);
        _display.println(lines[i].c_str());
    }
    
    _display.display();
}

void DisplayManager::displayMemoryInfo() 
{
    displayLines(MemoryManager::getStatusLines());
} 