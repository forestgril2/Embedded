#include "server_manager.h"
#include <ESP.h>

ServerManager::ServerManager(DisplayManager& display) 
    : server(80), display(display) {
}

void ServerManager::begin() {
    // Setup server routes
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleRoot(request);
    });
    
    server.on("/text", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleText(request);
    });
    
    server.on("/version", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleVersion(request);
    });
    
    server.on("/memory", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleMemoryStatus(request);
    });
    
    server.on("/debug", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleDebug(request);
    });

    server.begin();
}

void ServerManager::handleClient() {
    // AsyncWebServer handles clients automatically
}

void ServerManager::handleText(AsyncWebServerRequest *request) {
    if (request->hasParam("text", true)) {
        String text = request->getParam("text", true)->value();
        display.displayText(text.c_str());
        request->send(200, "text/plain", "Text displayed: " + text);
    } else {
        request->send(400, "text/plain", "Missing 'text' parameter");
    }
}

void ServerManager::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html",
                "<form action=\"/text\" method=\"POST\">"
                "Text to display: <input name=\"text\" type=\"text\">"
                "<input type=\"submit\" value=\"Display\">"
                "</form>");
}

void ServerManager::handleMemoryStatus(AsyncWebServerRequest *request) {
    char response[256];
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freePsram = ESP.getFreePsram();
    uint32_t totalPsram = ESP.getPsramSize();
    uint32_t freeSketchSpace = ESP.getFreeSketchSpace();
    uint32_t sketchSize = ESP.getSketchSize();
    
    sprintf(response, 
            "{\"heap\":{\"free\":%u,\"total\":%u},\"psram\":{\"free\":%u,\"total\":%u},\"flash\":{\"free\":%u,\"total\":%u}}",
            freeHeap, totalHeap, freePsram, totalPsram, freeSketchSpace, sketchSize);
    
    request->send(200, "application/json", response);
}

void ServerManager::handleVersion(AsyncWebServerRequest *request) {
    String versionInfo;
    #ifdef GIT_COMMIT_HASH
        versionInfo = "Firmware Info:\nCommit: " + String(GIT_COMMIT_HASH);
    #else
        versionInfo = "Firmware Info:\nCommit: unknown";
    #endif
    request->send(200, "text/plain", versionInfo);
}

void ServerManager::handleDebug(AsyncWebServerRequest *request) {
    String debugInfo = "Debug Info:\n";
    debugInfo += "WiFi Status: " + String(WiFi.status()) + "\n";
    debugInfo += "IP Address: " + WiFi.localIP().toString() + "\n";
    debugInfo += "MAC Address: " + WiFi.macAddress() + "\n";
    debugInfo += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    request->send(200, "text/plain", debugInfo);
} 