#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "display_manager.h"

class PinManager {
public:
    struct PinConfig {
        int8_t stepperStepPin;
        int8_t stepperDirPin;
        int8_t stepperEnablePin;
        int8_t displaySdaPin;
        int8_t displaySclPin;
        int8_t displayResetPin;
        int8_t ledPin;
    };

    // TODO: I2C pins (SDA/SCL) should not be configurable on ESP32 as they are fixed
    // TODO: Consider removing displayResetPin as it's not used (-1)
    // TODO: Consider adding validation for I2C pins to prevent configuration
    // TODO: Add method to validate entire config at once instead of individual pins
    // TODO: Add method to check if config is valid before saving
    // TODO: Consider adding a method to reset to defaults without saving to EEPROM
    // TODO: Add method to check if current config matches defaults

    static const int EEPROM_SIZE = 512;
    static const int CONFIG_ADDRESS = 0;
    static const uint32_t CONFIG_VERSION = 1;
    static const PinConfig DEFAULT_CONFIG;

    PinManager(DisplayManager& display) : display(display) {}
    bool init();
    void saveConfig(const PinConfig& config);
    PinConfig loadConfig();
    String getConfigJson();
    static bool validatePin(int8_t pin);
    void setDefaultConfig();

private:
    DisplayManager& display;
}; 