#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include <Arduino.h>

class LimitSwitch {
public:
    // Constructor
    LimitSwitch(uint8_t pin, const char* id, bool activeLow = true);
    
    // Core functionality
    bool init();
    bool isTriggered() const;
    const char* getId() const;
    uint8_t getPin() const;
    
    // Interrupt handling
    static void IRAM_ATTR handleInterrupt(void* arg);
    
private:
    uint8_t _pin;
    const char* _id;
    bool _activeLow;
    volatile bool _isTriggered;
    volatile unsigned long _lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50; // ms
};

#endif // LIMIT_SWITCH_H 