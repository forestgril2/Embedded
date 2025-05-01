#ifndef STEPPER_MANAGER_H
#define STEPPER_MANAGER_H

#include <AccelStepper.h>
#include "display_manager.h"

class StepperManager {
public:
    StepperManager(DisplayManager& display);
    void begin();
    void moveTo(long position);
    void run();
    void stop();
    void setSpeed(float speed);
    void setAcceleration(float acceleration);
    bool isRunning();

private:
    DisplayManager& _display;
    AccelStepper _stepper;
    static const int STEP_PIN = 13;    // GPIO13 for step signal
    static const int DIR_PIN = 14;     // GPIO14 for direction signal
    static const int ENABLE_PIN = 15;  // GPIO15 for enable signal
};

#endif // STEPPER_MANAGER_H 