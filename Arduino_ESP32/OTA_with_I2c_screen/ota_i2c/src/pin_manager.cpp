#include "pin_manager.h"

// Define static const member variables
const int PinManager::EEPROM_SIZE;
const int PinManager::CONFIG_ADDRESS;
const uint32_t PinManager::CONFIG_VERSION;

bool PinManager::init() {
    EEPROM.begin(EEPROM_SIZE);
    return true;
}

void PinManager::saveConfig(const PinConfig& config) {
    StaticJsonDocument<256> doc;
    doc["version"] = CONFIG_VERSION;
    doc["stepperStepPin"] = config.stepperStepPin;
    doc["stepperDirPin"] = config.stepperDirPin;
    doc["stepperEnablePin"] = config.stepperEnablePin;
    doc["displaySdaPin"] = config.displaySdaPin;
    doc["displaySclPin"] = config.displaySclPin;
    doc["displayResetPin"] = config.displayResetPin;
    doc["ledPin"] = config.ledPin;

    String jsonString;
    serializeJson(doc, jsonString);
    
    // Clear the EEPROM section
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(CONFIG_ADDRESS + i, 0);
    }
    
    // Write the JSON string
    for (size_t i = 0; i < jsonString.length(); i++) {
        EEPROM.write(CONFIG_ADDRESS + i, jsonString[i]);
    }
    
    EEPROM.commit();
}

PinManager::PinConfig PinManager::loadConfig() {
    PinConfig config;
    String jsonString;
    
    // Read the JSON string from EEPROM
    for (int i = 0; i < EEPROM_SIZE; i++) {
        char c = EEPROM.read(CONFIG_ADDRESS + i);
        if (c == 0) break;
        jsonString += c;
    }
    
    if (jsonString.length() == 0) {
        setDefaultConfig();
        return loadConfig();
    }
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error || doc["version"] != CONFIG_VERSION) {
        setDefaultConfig();
        return loadConfig();
    }
    
    config.stepperStepPin = doc["stepperStepPin"] | -1;
    config.stepperDirPin = doc["stepperDirPin"] | -1;
    config.stepperEnablePin = doc["stepperEnablePin"] | -1;
    config.displaySdaPin = doc["displaySdaPin"] | -1;
    config.displaySclPin = doc["displaySclPin"] | -1;
    config.displayResetPin = doc["displayResetPin"] | -1;
    config.ledPin = doc["ledPin"] | -1;
    
    return config;
}

String PinManager::getConfigJson() {
    PinConfig config = loadConfig();
    StaticJsonDocument<256> doc;
    
    doc["stepperStepPin"] = config.stepperStepPin;
    doc["stepperDirPin"] = config.stepperDirPin;
    doc["stepperEnablePin"] = config.stepperEnablePin;
    doc["displaySdaPin"] = config.displaySdaPin;
    doc["displaySclPin"] = config.displaySclPin;
    doc["displayResetPin"] = config.displayResetPin;
    doc["ledPin"] = config.ledPin;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool PinManager::validatePin(int8_t pin) {
    // ESP32 has GPIO 0-39, or -1 for unused pins
    return pin == -1 || (pin >= 0 && pin <= 39);
}

void PinManager::setDefaultConfig() {
    PinConfig defaultConfig = {
        .stepperStepPin = 13,    // Original: GPIO13 for step signal
        .stepperDirPin = 14,     // Original: GPIO14 for direction signal
        .stepperEnablePin = 12,  // Original: GPIO12 for enable signal
        .displaySdaPin = 21,     // Original: Default I2C SDA pin
        .displaySclPin = 22,     // Original: Default I2C SCL pin
        .displayResetPin = -1,   // Original: No reset pin (-1)
        .ledPin = -1            // Original: LED pin not set (-1)
    };
    saveConfig(defaultConfig);
} 