#include "stepper_manager.h"
#include <FastAccelStepper.h>
#include <Arduino.h>

StepperManager::StepperManager(DisplayManager& display) :
    _display(display),
    _currentSpeed(6400), 
    _currentAcceleration(30000) 
{}

bool StepperManager::init() {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);  // Enable the stepper driver
    
    // Initialize the stepper engine
    _engine.init();
    
    // Create a stepper instance
    _stepper = _engine.stepperConnectToPin(STEP_PIN);
    if (_stepper) 
    {
        _stepper->setDirectionPin(DIR_PIN);
        _stepper->setEnablePin(ENABLE_PIN);
        _stepper->setAutoEnable(true);
        
        // Set default speed and acceleration
        _stepper->setSpeedInHz(_currentSpeed);
        _stepper->setAcceleration(_currentAcceleration);
        
        _display.displayText("Stepper initialized");
        return true;
    } 
    else 
    {
        _display.displayText("Stepper init failed");
        return false;
    }
}

void StepperManager::moveTo(long position) 
{
    if (_stepper) 
    {
        _stepper->moveTo(position);
    }
}

void StepperManager::run() { /* FastAccelStepper handles running automatically */ }

void StepperManager::stop() 
{
    if (_stepper) 
    {
        _stepper->stopMove();
    }
}

void StepperManager::setSpeed(float speed) 
{
    if (_stepper) 
    {
        _currentSpeed = speed;
        _stepper->setSpeedInHz(speed);
    }
}

void StepperManager::setAcceleration(float acceleration) 
{
    if (_stepper) 
    {
        _currentAcceleration = acceleration;
        _stepper->setAcceleration(acceleration);
    }
}

bool StepperManager::isRunning() 
{
    if (_stepper) 
    {
        return _stepper->isRunning();
    }
    return false;
}

long StepperManager::getCurrentPosition() 
{
    if (_stepper) 
    {
        return _stepper->getCurrentPosition();
    }
    return 0;
}

float StepperManager::getCurrentSpeed() 
{
    if (!_stepper) return 0.0f;

    // Get current position and time
    long currentPosition = _stepper->getCurrentPosition();
    unsigned long currentTime = millis();
    
    // Calculate speed if we have previous position data
    if (_lastPositionTime > 0) {
        float timeDelta = (currentTime - _lastPositionTime) / 1000.0f; // Convert to seconds
        if (timeDelta > 0) {
            _calculatedSpeed = (currentPosition - _lastPosition) / timeDelta; // steps per second
        }
    }
    
    // Update last position and time
    _lastPosition = currentPosition;
    _lastPositionTime = currentTime;
    
    return _calculatedSpeed;
}

float StepperManager::getCurrentAcceleration() 
{
    return _currentAcceleration;
}

int StepperManager::getMicrosteps() 
{
    return MICROSTEPS;
} 