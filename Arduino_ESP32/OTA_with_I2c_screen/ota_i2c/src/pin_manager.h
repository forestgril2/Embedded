#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "display_manager.h"
#include "flash_controller.h"

class PinManager {
public:
    using PinConfig = FlashController::PinConfig;

    static const PinConfig DEFAULT_CONFIG;

    PinManager(DisplayManager& display) : display(display) {}
    
    bool init() {
        Serial.println("PinManager::init() called");
        return FlashController::init();
    }
    
    void saveConfig(const PinConfig& config) {
        Serial.println("PinManager::saveConfig() called");
        config.print();
        if (!FlashController::writePinConfig(config)) {
            Serial.println("Failed to save pin configuration");
        }
    }
    
    PinConfig loadConfig() {
        Serial.println("PinManager::loadConfig() called");
        PinConfig config;
        if (!FlashController::readPinConfig(config)) {
            Serial.println("Failed to read pin configuration, using defaults");
            config = DEFAULT_CONFIG;
            saveConfig(config);
        }
        config.print();
        return config;
    }
    
    String getConfigJson() {
        PinConfig config = loadConfig();
        StaticJsonDocument<256> doc;
        
        doc["stepperStepPin"] = config.stepperStepPin;
        doc["stepperDirPin"] = config.stepperDirPin;
        doc["stepperEnablePin"] = config.stepperEnablePin;
        doc["displaySdaPin"] = config.displaySdaPin;
        doc["displaySclPin"] = config.displaySclPin;
        doc["displayResetPin"] = config.displayResetPin;
        doc["ledPin"] = config.ledPin;
        
        String output;
        serializeJson(doc, output);
        return output;
    }
    
    static bool validatePin(int8_t pin) {
        return pin > 0 && pin < 40;
    }
    
    void setDefaultConfig() {
        Serial.println("Setting default pin configuration");
        saveConfig(DEFAULT_CONFIG);
    }

private:
    DisplayManager& display;
}; 