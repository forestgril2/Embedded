#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <EEPROM.h>

class LedControl {
public:
    static const int DEFAULT_LED_PIN = 1;  // Default LED pin
    static const int LED_PIN_EEPROM_ADDR = 0;  // EEPROM address to store LED pin
    static const bool LED_ACTIVE_LOW = true;  // Set to true if LED is active low

    LedControl(int pin) : _pin(pin) {}
    
    void begin() {
        pinMode(_pin, OUTPUT);
        setLedOff();  // Start with LED off
    }
    
    void blink(int count, int delayMs) {
        for (int i = 0; i < count; i++) {
            setLedOn();
            delay(delayMs);
            setLedOff();
            delay(delayMs);
        }
    }
    
    void on() {
        setLedOn();
    }
    
    void off() {
        setLedOff();
    }

    // Static method to get LED pin from EEPROM or return default
    static int getLedPin() {
        static bool eepromInitialized = false;
        if (!eepromInitialized) {
            EEPROM.begin(1);  // Initialize EEPROM with 1 byte
            eepromInitialized = true;
        }
        
        int pin = EEPROM.read(LED_PIN_EEPROM_ADDR);
        // If pin is 0 or invalid, return default
        if (pin <= 0 || pin >= 40) {
            pin = DEFAULT_LED_PIN;
            // Save default to EEPROM
            EEPROM.write(LED_PIN_EEPROM_ADDR, pin);
            EEPROM.commit();
        }
        return pin;
    }

    // Static method to save LED pin to EEPROM
    static void saveLedPin(int pin) {
        if (pin > 0 && pin < 40) {  // Validate pin number
            static bool eepromInitialized = false;
            if (!eepromInitialized) {
                EEPROM.begin(1);
                eepromInitialized = true;
            }
            EEPROM.write(LED_PIN_EEPROM_ADDR, pin);
            EEPROM.commit();
        }
    }

private:
    int _pin;

    void setLedOn() {
        digitalWrite(_pin, LED_ACTIVE_LOW ? LOW : HIGH);
    }

    void setLedOff() {
        digitalWrite(_pin, LED_ACTIVE_LOW ? HIGH : LOW);
    }
};

#endif // LED_CONTROL_H 