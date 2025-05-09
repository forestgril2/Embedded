#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "flash_controller.h"

class LedControl 
{
public:
    static const int DEFAULT_LED_PIN = 1;  // Default LED pin
    static const bool LED_ACTIVE_LOW = true;  // Set to true if LED is active low

    LedControl(int pin = DEFAULT_LED_PIN) : _pin(pin) 
    {
    }
    
    void init() 
    {
        pinMode(_pin, OUTPUT);
        setLedOff();  // Start with LED off
    }
    
    void blink(int count, int delayMs) 
    {
        for (int i = 0; i < count; i++) 
        {
            setLedOn();
            delay(delayMs);
            setLedOff();
            delay(delayMs);
        }
    }

    void on() 
    {
        setLedOn();
    }
    
    void off() 
    {
        setLedOff();
    }

    // Static method to get LED pin from Flash or return default
    static int getLedPin() 
    {
        Serial.println("LedControl::getLedPin() called");
        int pin = FlashController::readLedPin();
        if (pin < 0) {
            Serial.println("Failed to read LED pin from Flash, using default");
            pin = DEFAULT_LED_PIN;
            FlashController::writeLedPin(pin);
        }
        return pin;
    }

    // Static method to save LED pin to Flash
    static void saveLedPin(int pin) 
    {
        Serial.printf("LedControl::saveLedPin() called with pin: %d\n", pin);
        if (!FlashController::writeLedPin(pin)) {
            Serial.printf("Failed to save LED pin %d to Flash\n", pin);
        }
    }

private:
    int _pin;

    void setLedOn() 
    {
        digitalWrite(_pin, LED_ACTIVE_LOW ? LOW : HIGH);
    }

    void setLedOff() 
    {
        digitalWrite(_pin, LED_ACTIVE_LOW ? HIGH : LOW);
    }
};

#endif // LED_CONTROL_H 