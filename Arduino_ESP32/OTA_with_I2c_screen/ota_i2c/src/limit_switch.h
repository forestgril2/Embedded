#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include <Arduino.h>

class LimitSwitch {
public:
    // Priority levels for different types of switches
    static const uint8_t PRIORITY_CRITICAL = 1;  // Highest priority
    static const uint8_t PRIORITY_HIGH = 2;
    static const uint8_t PRIORITY_NORMAL = 3;
    static const uint8_t PRIORITY_LOW = 4;
    static const uint8_t PRIORITY_LOWEST = 7;    // Lowest priority

    // Constructor
    LimitSwitch(uint8_t pin, const char* id, bool activeLow = true, uint8_t priority = PRIORITY_NORMAL);
    
    // Core functionality
    bool init();
    bool isTriggered() const;
    const char* getId() const;
    uint8_t getPin() const;
    void update();  // New method to handle state changes
    
    // Interrupt handling
    static void IRAM_ATTR handleInterrupt(void* arg);
    
private:
    uint8_t _pin;
    const char* _id;
    bool _activeLow;
    uint8_t _priority;  // Interrupt priority (1-7)
    volatile bool _isTriggered;
    volatile bool _stateChanged;  // New flag for state changes
    volatile unsigned long _lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50; // ms
};

#endif // LIMIT_SWITCH_H 