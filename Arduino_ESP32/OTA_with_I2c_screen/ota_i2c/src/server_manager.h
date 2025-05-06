#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include "display_manager.h"
#include "stepper_manager.h"

class ServerManager 
{
public:
    ServerManager(DisplayManager& display, StepperManager& stepper);
    void init();
    bool isInitialized() const { return _initialized; }
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
    void handleLedTest(AsyncWebServerRequest *request);
    void handleLedPinConfig(AsyncWebServerRequest *request);
    void handleWifiReset(AsyncWebServerRequest *request);

private:
    AsyncWebServer server;
    DisplayManager& display;
    StepperManager& stepper;
    bool _initialized = false;
};

#endif // SERVER_MANAGER_H 