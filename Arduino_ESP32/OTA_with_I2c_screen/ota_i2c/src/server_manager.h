#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
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
    
    // WebSocket methods
    void broadcastStatus();
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    DisplayManager& display;
    StepperManager& stepper;
    bool _initialized = false;
    unsigned long _lastStatusUpdate = 0;
    const unsigned long STATUS_UPDATE_INTERVAL = 250; // Update every 250ms
    
    // Speed calculation variables
    long _lastPosition = 0;
    unsigned long _lastPositionTime = 0;
    float _calculatedSpeed = 0.0f;

    // HTML generation methods
    String generateHeader();
    String generateTextDisplayForm();
    String generateStepperControlForms();
    String generateLedControlForms();
    String generateWifiControlForm();
    String generateFooter();
};

#endif // SERVER_MANAGER_H 