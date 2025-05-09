#ifndef CONTROL_SIGNAL_HANDLER_H
#define CONTROL_SIGNAL_HANDLER_H

#include <vector>
#include <functional>
#include <memory>  // For shared_ptr
#include "limit_switch.h"
#include "display_manager.h"

class ControlSignalHandler {
public:
    // Signal handler function type
    using SignalHandlerFunc = std::function<void(const char*)>;
    
    ControlSignalHandler(DisplayManager& display);
    // No need for explicit destructor with shared_ptr
    
    bool init();
    void handle();
    
    // Limit switch management
    bool addLimitSwitch(uint8_t pin, const char* id, bool activeLow = true);
    void removeLimitSwitch(const char* id);
    bool isLimitSwitchTriggered(const char* id) const;
    
    // Signal handler registration
    void registerSignalHandler(SignalHandlerFunc handler);
    
private:
    DisplayManager& _display;
    std::vector<std::shared_ptr<LimitSwitch>> _limitSwitches;  // Using shared_ptr
    SignalHandlerFunc _signalHandler;
    bool _initialized;
    
    void onSwitchActivated(const char* switchId);
};

#endif // CONTROL_SIGNAL_HANDLER_H 