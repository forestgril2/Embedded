#include "stepper_manager.h"
#include <FastAccelStepper.h>
#include <Arduino.h>

StepperManager::StepperManager(DisplayManager& display)
    : _display(display) {
}

void StepperManager::begin() {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);  // Enable the stepper driver
    
    // Initialize the stepper engine
    _engine.init();
    
    // Create a stepper instance
    _stepper = _engine.stepperConnectToPin(STEP_PIN);
    if (_stepper) {
        _stepper->setDirectionPin(DIR_PIN);
        _stepper->setEnablePin(ENABLE_PIN);
        _stepper->setAutoEnable(true);
        
        // Set default speed and acceleration
        _stepper->setSpeedInHz(1000);  // steps per second
        _stepper->setAcceleration(500);  // steps per second squared
        
        _display.displayText("Stepper initialized");
    } else {
        _display.displayText("Stepper init failed");
    }
}

void StepperManager::moveTo(long position) {
    if (_stepper) {
        _stepper->moveTo(position);
    }
}

void StepperManager::run() {
    // FastAccelStepper handles running automatically
}

void StepperManager::stop() {
    if (_stepper) {
        _stepper->stopMove();
    }
}

void StepperManager::setSpeed(float speed) {
    if (_stepper) {
        _stepper->setSpeedInHz(speed);
    }
}

void StepperManager::setAcceleration(float acceleration) {
    if (_stepper) {
        _stepper->setAcceleration(acceleration);
    }
}

bool StepperManager::isRunning() {
    if (_stepper) {
        return _stepper->isRunning();
    }
    return false;
} 