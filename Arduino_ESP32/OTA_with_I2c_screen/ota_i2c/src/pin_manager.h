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