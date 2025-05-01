#include "ota_manager.h"
#include <ArduinoOTA.h>

OTAManager::OTAManager(Adafruit_SSD1306& display) : _display(display) {}

void OTAManager::begin(const char* hostname, const char* password) {
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword(password);
    
    ArduinoOTA.onStart([this]() { this->onStart(); });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) { 
        this->onProgress(progress, total); 
    });
    ArduinoOTA.onEnd([this]() { this->onEnd(); });
    ArduinoOTA.onError([this](ota_error_t error) { this->onError(error); });

    ArduinoOTA.begin();
}

void OTAManager::handle() {
    ArduinoOTA.handle();
}

void OTAManager::onStart() {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    _display.println("OTA Update Start");
    _display.display();
}

void OTAManager::onProgress(unsigned int progress, unsigned int total) {
    char progressStr[32];
    sprintf(progressStr, "Progress: %u%%", (progress / (total / 100)));
    
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    _display.println(progressStr);
    _display.display();
}

void OTAManager::onEnd() {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    _display.println("OTA Update Complete");
    _display.setCursor(0, 10);
    _display.println("Please reset");
    _display.setCursor(0, 20);
    _display.println("the device");
    _display.display();
    
    delay(5000);
    ESP.restart();
}

void OTAManager::onError(ota_error_t error) {
    char errorStr[32];
    sprintf(errorStr, "Error[%u]: ", error);
    
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.setTextColor(SSD1306_WHITE);
    _display.println(errorStr);
    _display.display();
} 