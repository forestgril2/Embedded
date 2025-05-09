#include "limit_switch.h"

LimitSwitch::LimitSwitch(uint8_t pin, const char* id, bool activeLow, uint8_t priority)
    : _pin(pin), _id(id), _activeLow(activeLow), _priority(priority),
      _isTriggered(false), _stateChanged(false), _lastDebounceTime(0) {}

bool LimitSwitch::init() {
    if (_pin > 39) return false;  // ESP32 has GPIO 0-39
    
    pinMode(_pin, INPUT_PULLUP);
    Serial.printf("Setting up interrupt for pin %d, id: %s, priority: %d\n", 
                 _pin, _id, _priority);
    
    // On ESP32, we can use the GPIO number directly for interrupts
    // Set the interrupt priority
    esp_intr_alloc(ESP_INTR_FLAG_IRAM, _priority, handleInterrupt, this, NULL);
    attachInterruptArg(_pin, handleInterrupt, this, CHANGE);
    return true;
}

void IRAM_ATTR LimitSwitch::handleInterrupt(void* arg) {
    LimitSwitch* sw = static_cast<LimitSwitch*>(arg);
    if (!sw) return;  // Safety check
    
    // Just set the flag - do minimal work in ISR
    sw->_stateChanged = true;
}

void LimitSwitch::update() {
    if (_stateChanged) {
        unsigned long currentTime = millis();
        if (currentTime - _lastDebounceTime > DEBOUNCE_DELAY) {
            bool currentState = digitalRead(_pin);
            if (_activeLow) {
                currentState = !currentState;
            }
            
            if (currentState != _isTriggered) {
                _isTriggered = currentState;
                Serial.printf("Switch %s on pin %d changed state to: %d\n", 
                            _id, _pin, currentState);
            }
            _lastDebounceTime = currentTime;
            _stateChanged = false;  // Only clear the flag after processing
        }
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