#include "stepper_manager.h"
#include <AccelStepper.h>

StepperManager::StepperManager(DisplayManager& display)
    : _display(display),
      _stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN) {
}

void StepperManager::begin() {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);  // Enable the stepper driver
    
    // Set default speed and acceleration
    _stepper.setMaxSpeed(1000.0);
    _stepper.setAcceleration(500.0);
    
    _display.displayText("Stepper initialized");
}

void StepperManager::moveTo(long position) {
    _stepper.moveTo(position);
}

void StepperManager::run() {
    _stepper.run();
}

void StepperManager::stop() {
    _stepper.stop();
    digitalWrite(ENABLE_PIN, HIGH);  // Disable the stepper driver
}

void StepperManager::setSpeed(float speed) {
    _stepper.setMaxSpeed(speed);
}

void StepperManager::setAcceleration(float acceleration) {
    _stepper.setAcceleration(acceleration);
}

bool StepperManager::isRunning() {
    return _stepper.isRunning();
} 