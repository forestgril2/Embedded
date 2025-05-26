#ifndef FLASH_CONTROLLER_H
#define FLASH_CONTROLLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

class FlashController {
public:
    // Memory layout constants
    static const int TOTAL_FLASH_SIZE = 512;  // Total Flash size in bytes
    
    // Address ranges for different components
    static const int PIN_CONFIG_START = 0;     // Pin configuration starts at 0
    static const int PIN_CONFIG_SIZE = 32;     // Reserve 32 bytes for pin config
    
    // Pin configuration addresses
    static const int LED_PIN_ADDR = PIN_CONFIG_START + PIN_CONFIG_SIZE;
    static const int LED_PIN_SIZE = 1;
    
    static const int STEPPER_STEP_PIN_ADDR = LED_PIN_ADDR + LED_PIN_SIZE;
    static const int STEPPER_DIR_PIN_ADDR = STEPPER_STEP_PIN_ADDR + 1;
    static const int STEPPER_ENABLE_PIN_ADDR = STEPPER_DIR_PIN_ADDR + 1;
    static const int DISPLAY_SDA_PIN_ADDR = STEPPER_ENABLE_PIN_ADDR + 1;
    static const int DISPLAY_SCL_PIN_ADDR = DISPLAY_SDA_PIN_ADDR + 1;
    static const int DISPLAY_RESET_PIN_ADDR = DISPLAY_SCL_PIN_ADDR + 1;

    // Debug logging levels
    enum class LogLevel {
        NONE = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4
    };

private:
    static bool _initialized;
    static LogLevel _logLevel;
    static const char* _logPrefix;

    static void log(LogLevel level, const char* format, ...) {
        if (level <= _logLevel) {
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            Serial.printf("[%s] %s\n", _logPrefix, buffer);
        }
    }

public:
    static void setLogLevel(LogLevel level) {
        _logLevel = level;
    }

    static bool init() {
        if (!_initialized) {
            log(LogLevel::INFO, "Initializing Flash controller");
            EEPROM.begin(TOTAL_FLASH_SIZE);
            _initialized = true;
        }
        return _initialized;
    }

    static void commit() {
        if (_initialized) {
            EEPROM.commit();
            log(LogLevel::DEBUG, "Flash changes committed");
        }
    }

    // LED pin methods
    static int8_t readLedPin() {
        if (!_initialized) return -1;
        
        int8_t pin = EEPROM.read(LED_PIN_ADDR);
        log(LogLevel::DEBUG, "Read LED pin from Flash: %d", pin);
        
        if (pin <= 0 || pin >= 40) {
            log(LogLevel::WARN, "Invalid LED pin %d, using default", pin);
            pin = 1;  // Default LED pin
            writeLedPin(pin);
        }
        return pin;
    }

    static bool writeLedPin(int8_t pin) {
        if (!_initialized) return false;
        
        if (pin > 0 && pin < 40) {
            log(LogLevel::DEBUG, "Writing LED pin to Flash: %d", pin);
            EEPROM.write(LED_PIN_ADDR, pin);
            commit();
            return true;
        }
        log(LogLevel::ERROR, "Invalid LED pin value: %d", pin);
        return false;
    }

    // PinManager specific methods
    struct PinConfig {
        int8_t stepperStepPin;
        int8_t stepperDirPin;
        int8_t stepperEnablePin;
        int8_t displaySdaPin;
        int8_t displaySclPin;
        int8_t displayResetPin;
        int8_t ledPin;

        void print() const {
            Serial.printf("PinConfig: stepperStep=%d, stepperDir=%d, stepperEnable=%d, "
                        "displaySda=%d, displayScl=%d, displayReset=%d, led=%d\n",
                        stepperStepPin, stepperDirPin, stepperEnablePin,
                        displaySdaPin, displaySclPin, displayResetPin, ledPin);
        }
    };

    static bool readPinConfig(PinConfig& config) {
        if (!_initialized) return false;
        
        log(LogLevel::DEBUG, "Reading pin configuration");
        
        config.stepperStepPin = EEPROM.read(STEPPER_STEP_PIN_ADDR);
        config.stepperDirPin = EEPROM.read(STEPPER_DIR_PIN_ADDR);
        config.stepperEnablePin = EEPROM.read(STEPPER_ENABLE_PIN_ADDR);
        config.displaySdaPin = EEPROM.read(DISPLAY_SDA_PIN_ADDR);
        config.displaySclPin = EEPROM.read(DISPLAY_SCL_PIN_ADDR);
        config.displayResetPin = EEPROM.read(DISPLAY_RESET_PIN_ADDR);
        config.ledPin = EEPROM.read(LED_PIN_ADDR);

        log(LogLevel::DEBUG, "Pin config read: stepperStep=%d, stepperDir=%d, stepperEnable=%d, "
            "displaySda=%d, displayScl=%d, displayReset=%d, led=%d",
            config.stepperStepPin, config.stepperDirPin, config.stepperEnablePin,
            config.displaySdaPin, config.displaySclPin, config.displayResetPin, config.ledPin);
        
        return true;
    }

    static bool writePinConfig(const PinConfig& config) {
        if (!_initialized) return false;
        
        log(LogLevel::DEBUG, "Writing pin configuration");
        
        EEPROM.write(STEPPER_STEP_PIN_ADDR, config.stepperStepPin);
        EEPROM.write(STEPPER_DIR_PIN_ADDR, config.stepperDirPin);
        EEPROM.write(STEPPER_ENABLE_PIN_ADDR, config.stepperEnablePin);
        EEPROM.write(DISPLAY_SDA_PIN_ADDR, config.displaySdaPin);
        EEPROM.write(DISPLAY_SCL_PIN_ADDR, config.displaySclPin);
        EEPROM.write(DISPLAY_RESET_PIN_ADDR, config.displayResetPin);
        EEPROM.write(LED_PIN_ADDR, config.ledPin);
        
        commit();
        
        log(LogLevel::DEBUG, "Pin config written: stepperStep=%d, stepperDir=%d, stepperEnable=%d, "
            "displaySda=%d, displayScl=%d, displayReset=%d, led=%d",
            config.stepperStepPin, config.stepperDirPin, config.stepperEnablePin,
            config.displaySdaPin, config.displaySclPin, config.displayResetPin, config.ledPin);
        
        return true;
    }

    // Generic read/write methods
    template<typename T>
    static bool read(int address, T& value) {
        if (!_initialized) return false;
        if (address < 0 || address + sizeof(T) > TOTAL_FLASH_SIZE) {
            log(LogLevel::ERROR, "Invalid address range for read: %d, size: %d", 
                address, sizeof(T));
            return false;
        }
        
        EEPROM.get(address, value);
        log(LogLevel::DEBUG, "Read value at address %d", address);
        return true;
    }

    template<typename T>
    static bool write(int address, const T& value) {
        if (!_initialized) return false;
        if (address < 0 || address + sizeof(T) > TOTAL_FLASH_SIZE) {
            log(LogLevel::ERROR, "Invalid address range for write: %d, size: %d", 
                address, sizeof(T));
            return false;
        }
        
        EEPROM.put(address, value);
        commit();
        log(LogLevel::DEBUG, "Wrote value at address %d", address);
        return true;
    }
};

#endif // FLASH_CONTROLLER_H 