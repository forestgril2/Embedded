#ifndef STEPPER_MANAGER_H
#define STEPPER_MANAGER_H

#include <FastAccelStepper.h>
#include "display_manager.h"

class StepperManager 
{
public:
    StepperManager(DisplayManager& display);
    void begin();
    void moveTo(long position);
    void run();
    void stop();
    void setSpeed(float speed);
    void setAcceleration(float acceleration);
    bool isRunning();
    long getCurrentPosition();
    float getCurrentSpeed();
    float getCurrentAcceleration();
    int getMicrosteps();

private:
    DisplayManager& _display;
    FastAccelStepperEngine _engine;
    FastAccelStepper* _stepper;
    static const int STEP_PIN = 13;    // GPIO13 for step signal
    static const int DIR_PIN = 14;     // GPIO14 for direction signal
    static const int ENABLE_PIN = 12;  // GPIO12 for enable signal
    static const int MICROSTEPS = 4;   // 1/4 microstepping (800 steps/rev)
    float _currentSpeed;               // Target speed
    float _currentAcceleration;        // Current acceleration
    
    // Speed calculation variables
    long _lastPosition = 0;
    unsigned long _lastPositionTime = 0;
    float _calculatedSpeed = 0.0f;
};

#endif // STEPPER_MANAGER_H 