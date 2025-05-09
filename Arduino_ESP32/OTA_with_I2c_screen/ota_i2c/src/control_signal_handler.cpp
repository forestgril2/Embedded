#include "control_signal_handler.h"

ControlSignalHandler::ControlSignalHandler(DisplayManager& display)
    : _display(display), _initialized(false) {}

bool ControlSignalHandler::init() {
    _initialized = true;
    Serial.println("ControlSignalHandler initialized");
    return true;
}

void ControlSignalHandler::handle() {
    // Update all switches
    for (const auto& sw : _limitSwitches) {
        sw->update();
    }
}

bool ControlSignalHandler::addLimitSwitch(uint8_t pin, const char* id, bool activeLow) {
    // Check if ID already exists
    for (const auto& sw : _limitSwitches) {
        if (strcmp(sw->getId(), id) == 0) {
            Serial.printf("Switch with ID %s already exists\n", id);
            return false;
        }
    }
    
    // Determine priority based on switch ID
    uint8_t priority = LimitSwitch::PRIORITY_NORMAL;  // Default priority
    if (strstr(id, "HOME") || strstr(id, "LIMIT")) {
        priority = LimitSwitch::PRIORITY_CRITICAL;  // Home and limit switches are critical
    } else if (strstr(id, "EMERGENCY")) {
        priority = LimitSwitch::PRIORITY_HIGH;  // Emergency stops are high priority
    }
    
    // Create and initialize new switch
    auto newSwitch = std::make_shared<LimitSwitch>(pin, id, activeLow, priority);
    if (!newSwitch->init()) {
        Serial.printf("Failed to initialize switch %s on pin %d\n", id, pin);
        return false;
    }
    
    _limitSwitches.push_back(newSwitch);
    Serial.printf("Added switch %s on pin %d (activeLow: %d, priority: %d)\n", 
                 id, pin, activeLow, priority);
    return true;
}

void ControlSignalHandler::removeLimitSwitch(const char* id) {
    auto it = std::find_if(_limitSwitches.begin(), _limitSwitches.end(),
        [id](const std::shared_ptr<LimitSwitch>& sw) { 
            return strcmp(sw->getId(), id) == 0; 
        });
    
    if (it != _limitSwitches.end()) {
        _limitSwitches.erase(it);
    }
}

bool ControlSignalHandler::isLimitSwitchTriggered(const char* id) const {
    for (const auto& sw : _limitSwitches) {
        if (strcmp(sw->getId(), id) == 0) {
            return sw->isTriggered();
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