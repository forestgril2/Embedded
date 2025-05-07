#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "display_manager.h"
#include "stepper_manager.h"
#include "pin_manager.h"

class ServerManager 
{
public:
    // TODO: Pin configuration should be loaded at startup, not when visiting root path
    // TODO: Error messages should be shown on the web interface, not just the display
    // TODO: Add validation feedback on the web interface
    // TODO: Consider adding a separate endpoint for pin validation
    // TODO: Add method to check if pin changes require restart
    // TODO: Consider adding a warning when changing I2C pins
    // TODO: Add method to show current pin configuration status
    // TODO: Consider adding a way to reset to defaults from web interface
    // TODO: Add method to check if current config is valid before saving
    // TODO: Consider adding a way to backup/restore pin configuration

    ServerManager(DisplayManager& display, StepperManager& stepper, PinManager& pinManager);
    bool init();
    bool isInitialized() const { return _initialized; }
    void handleClient();
    void handleText(AsyncWebServerRequest *request);
    void handleRoot(AsyncWebServerRequest *request);
    void handleLedPage(AsyncWebServerRequest *request);
    void handlePinPage(AsyncWebServerRequest *request);
    void handleSystemPage(AsyncWebServerRequest *request);
    void handleMemoryStatus(AsyncWebServerRequest *request);
    void handleVersion(AsyncWebServerRequest *request);
    void handleDebug(AsyncWebServerRequest *request);
    void handleStepperMove(AsyncWebServerRequest *request);
    void handleStepperStop(AsyncWebServerRequest *request);
    void handleStepperSpeed(AsyncWebServerRequest *request);
    void handleStepperAccel(AsyncWebServerRequest *request);
    void handleStepperTorque(AsyncWebServerRequest *request);
    void handleLedTest(AsyncWebServerRequest *request);
    void handleLedPinConfig(AsyncWebServerRequest *request);
    void handleWifiReset(AsyncWebServerRequest *request);
    void handlePinConfig(AsyncWebServerRequest *request);
    void handlePinConfigGet(AsyncWebServerRequest *request);
    
    // WebSocket methods
    void broadcastStatus();
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    DisplayManager& display;
    StepperManager& stepper;
    PinManager& pinManager;
    bool _initialized = false;
    unsigned long _lastStatusUpdate = 0;
    const unsigned long STATUS_UPDATE_INTERVAL = 250; // Update every 250ms
    
    // WebSocket status tracking
    bool _wsConnected = false;
    unsigned long _lastWsReconnectAttempt = 0;
    const unsigned long WS_RECONNECT_INTERVAL = 5000; // Try to reconnect every 5 seconds
    
    // Speed calculation variables
    long _lastPosition = 0;
    unsigned long _lastPositionTime = 0;
    float _calculatedSpeed = 0.0f;
    long _targetPosition = 0;  // Track the last set target position

    // Helper methods
    void sendJsonResponse(AsyncWebServerRequest *request, int code, bool success, const char* error = nullptr);
    template<typename T>
    void sendJsonResponse(AsyncWebServerRequest *request, int code, bool success, const char* key, T value, const char* error = nullptr);

    // HTML generation methods
    String generateHeader();
    String generateTextDisplayForm();
    String generateStepperControlForms();
    String generateLedControlForms();
    String generateWifiControlForm();
    String generatePinConfigForm();
    String generateFooter();
};

#endif // SERVER_MANAGER_H 