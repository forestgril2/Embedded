#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include "display_manager.h"
#include "stepper_manager.h"

class ServerManager {
public:
    ServerManager(DisplayManager& display, StepperManager& stepper);
    void begin();
    void handleClient();
    void handleText(AsyncWebServerRequest *request);
    void handleRoot(AsyncWebServerRequest *request);
    void handleMemoryStatus(AsyncWebServerRequest *request);
    void handleVersion(AsyncWebServerRequest *request);
    void handleDebug(AsyncWebServerRequest *request);
    void handleStepperMove(AsyncWebServerRequest *request);
    void handleStepperStop(AsyncWebServerRequest *request);
    void handleStepperSpeed(AsyncWebServerRequest *request);
    void handleStepperAccel(AsyncWebServerRequest *request);

private:
    AsyncWebServer server;
    DisplayManager& display;
    StepperManager& stepper;
};

#endif // SERVER_MANAGER_H 