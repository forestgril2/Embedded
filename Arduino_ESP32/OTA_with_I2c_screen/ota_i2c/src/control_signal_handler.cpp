#include "control_signal_handler.h"

ControlSignalHandler::ControlSignalHandler(DisplayManager& display)
    : _display(display), _initialized(false) {}

bool ControlSignalHandler::init() {
    _initialized = true;
    Serial.println("ControlSignalHandler initialized");
    return true;
}

void ControlSignalHandler::handle() {
    // No polling needed - switches are handled by interrupts
}

bool ControlSignalHandler::addLimitSwitch(uint8_t pin, const char* id, bool activeLow) {
    // Check if ID already exists
    for (const auto& sw : _limitSwitches) {
        if (strcmp(sw.getId(), id) == 0) {
            Serial.printf("Switch with ID %s already exists\n", id);
            return false;
        }
    }
    
    // Create and initialize new switch
    LimitSwitch newSwitch(pin, id, activeLow);
    if (!newSwitch.init()) {
        Serial.printf("Failed to initialize switch %s on pin %d\n", id, pin);
        return false;
    }
    
    _limitSwitches.push_back(newSwitch);
    Serial.printf("Added switch %s on pin %d (activeLow: %d)\n", id, pin, activeLow);
    return true;
}

void ControlSignalHandler::removeLimitSwitch(const char* id) {
    _limitSwitches.erase(
        std::remove_if(_limitSwitches.begin(), _limitSwitches.end(),
            [id](const LimitSwitch& sw) { return strcmp(sw.getId(), id) == 0; }),
        _limitSwitches.end()
    );
}

bool ControlSignalHandler::isLimitSwitchTriggered(const char* id) const {
    for (const auto& sw : _limitSwitches) {
        if (strcmp(sw.getId(), id) == 0) {
            return sw.isTriggered();
        }
    }
    return false;
}

void ControlSignalHandler::registerSignalHandler(SignalHandlerFunc handler) {
    _signalHandler = handler;
    Serial.println("Signal handler registered");
}

void ControlSignalHandler::onSwitchActivated(const char* switchId) {
    Serial.printf("Switch activated: %s\n", switchId);
    if (_signalHandler) {
        _signalHandler(switchId);
    }
} 