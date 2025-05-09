#include "limit_switch.h"

LimitSwitch::LimitSwitch(uint8_t pin, const char* id, bool activeLow)
    : _pin(pin), _id(id), _activeLow(activeLow), _isTriggered(false), _lastDebounceTime(0) {}

bool LimitSwitch::init() {
    if (_pin > 39) return false;  // ESP32 has GPIO 0-39
    
    pinMode(_pin, INPUT_PULLUP);
    Serial.printf("Setting up interrupt for pin %d, id: %s\n", _pin, _id);
    attachInterruptArg(digitalPinToInterrupt(_pin), handleInterrupt, this, CHANGE);
    return true;
}

void IRAM_ATTR LimitSwitch::handleInterrupt(void* arg) {
    LimitSwitch* sw = static_cast<LimitSwitch*>(arg);
    unsigned long currentTime = millis();
    
    if (currentTime - sw->_lastDebounceTime > DEBOUNCE_DELAY) {
        bool currentState = digitalRead(sw->_pin);
        if (sw->_activeLow) {
            currentState = !currentState;
        }
        sw->_isTriggered = currentState;
        sw->_lastDebounceTime = currentTime;
        
        // Note: Serial prints in ISR are not recommended, but for debugging...
        Serial.printf("Interrupt on pin %d, state: %d\n", sw->_pin, currentState);
    }
}

bool LimitSwitch::isTriggered() const {
    return _isTriggered;
}

const char* LimitSwitch::getId() const {
    return _id;
}

uint8_t LimitSwitch::getPin() const {
    return _pin;
} 