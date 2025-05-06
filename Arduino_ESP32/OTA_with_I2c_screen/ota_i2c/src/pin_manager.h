#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

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

    static bool init();
    static void saveConfig(const PinConfig& config);
    static PinConfig loadConfig();
    static String getConfigJson();
    static bool validatePin(int8_t pin);
    static void setDefaultConfig();

private:
    static const int EEPROM_SIZE = 512;
    static const int CONFIG_ADDRESS = 0;
    static const uint32_t CONFIG_VERSION = 1;  // For future config structure changes
};

#endif // PIN_MANAGER_H 