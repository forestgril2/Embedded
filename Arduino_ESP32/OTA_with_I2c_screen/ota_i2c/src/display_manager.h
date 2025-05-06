#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>
#include <Arduino.h>

class DisplayManager 
{
public:
    DisplayManager(int width, int height, int resetPin = -1);
    bool begin(uint8_t i2cAddress = 0x3C);
    bool isInitialized() const { return _initialized; }
    void clear();
    void display();
    
    void displayText(const char* text, int line = 0);
    void displayLines(const std::vector<String>& lines);
    void displayMemoryInfo();

private:
    Adafruit_SSD1306 _display;
    bool _initialized = false;
    void _setupDisplay();
    void _setupTextDisplay();
};

#endif // DISPLAY_MANAGER_H 