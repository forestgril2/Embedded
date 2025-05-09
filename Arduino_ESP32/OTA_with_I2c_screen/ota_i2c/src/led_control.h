#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "flash_controller.h"

class LedControl 
{
public:
    static const int DEFAULT_LED_PIN = 2;  // Changed from 1 to 2 to avoid UART TX pin
    static const bool LED_ACTIVE_LOW = true;  // Set to true if LED is active low

    LedControl(int pin = DEFAULT_LED_PIN) : _pin(pin) 
    {
    }
    
    void init() 
    {
        Serial.println("LedControl::init() called");
        Serial.flush();
        pinMode(_pin, OUTPUT);
        Serial.printf("LED pin mode %d, output\n", _pin, OUTPUT);
        Serial.flush();
        setLedOff();  // Start with LED off
        Serial.printf("LED initialized on pin %d\n", _pin);
        Serial.flush();
    }
    
    void blink(int count, int delayMs) 
    {
        Serial.printf("LedControl::blink() called with count: %d, delay: %dms\n", count, delayMs);
        for (int i = 0; i < count; i++) 
        {
            setLedOn();
            delay(delayMs);
            setLedOff();
            delay(delayMs);
        }
        Serial.println("Blink sequence completed");
    }

    void on() 
    {
        Serial.println("LedControl::on() called");
        setLedOn();
    }
    
    void off() 
    {
        Serial.println("LedControl::off() called");
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