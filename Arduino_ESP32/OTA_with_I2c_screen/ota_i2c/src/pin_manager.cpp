#include "pin_manager.h"

// Define the default configuration
const PinManager::PinConfig PinManager::DEFAULT_CONFIG = {
    .stepperStepPin = 12,    // Example default values
    .stepperDirPin = 13,
    .stepperEnablePin = 14,
    .displaySdaPin = 21,     // ESP32 default I2C pins
    .displaySclPin = 22,     // ESP32 default I2C pins
    .displayResetPin = -1,   // Not used
    .ledPin = 1             // Default LED pin
};

// All method implementations have been moved to the header file as inline methods 