#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
public:
    DisplayManager(int width, int height, int resetPin = -1);
    bool begin(uint8_t i2cAddress = 0x3C);
    void clear();
    void display();
    
    void displayText(const char* text, int line = 0);
    void displayLines(const char* line1, const char* line2 = "", const char* line3 = "");
    void displayMemoryInfo();

private:
    Adafruit_SSD1306 _display;
    void _setupDisplay();
};

#endif // DISPLAY_MANAGER_H 