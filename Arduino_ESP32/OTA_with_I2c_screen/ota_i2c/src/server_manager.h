#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include "display_manager.h"

class ServerManager {
public:
    ServerManager(DisplayManager& display);
    void begin();
    void handleClient();

private:
    AsyncWebServer server;
    DisplayManager& display;

    void handleText(AsyncWebServerRequest *request);
    void handleRoot(AsyncWebServerRequest *request);
    void handleMemoryStatus(AsyncWebServerRequest *request);
    void handleVersion(AsyncWebServerRequest *request);
    void handleDebug(AsyncWebServerRequest *request);
};

#endif // SERVER_MANAGER_H 