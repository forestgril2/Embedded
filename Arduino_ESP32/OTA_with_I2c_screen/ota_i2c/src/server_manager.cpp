#include "server_manager.h"
#include <ESP.h>

ServerManager::ServerManager(DisplayManager& display, StepperManager& stepper) 
    : server(80), display(display), stepper(stepper) {
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

    // Stepper motor control endpoints
    server.on("/stepper/move", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleStepperMove(request);
    });

    server.on("/stepper/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleStepperStop(request);
    });

    server.on("/stepper/speed", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleStepperSpeed(request);
    });

    server.on("/stepper/accel", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleStepperAccel(request);
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
    String html = "<html><body>";
    html += "<h1>ESP32 Stepper Motor Control</h1>";
    html += "<form action=\"/text\" method=\"POST\">";
    html += "Text to display: <input name=\"text\" type=\"text\">";
    html += "<input type=\"submit\" value=\"Display\">";
    html += "</form>";
    html += "<h2>Stepper Motor Control</h2>";
    html += "<form action=\"/stepper/move\" method=\"POST\">";
    html += "Position: <input name=\"position\" type=\"number\">";
    html += "<input type=\"submit\" value=\"Move\">";
    html += "</form>";
    html += "<form action=\"/stepper/speed\" method=\"POST\">";
    html += "Speed: <input name=\"speed\" type=\"number\">";
    html += "<input type=\"submit\" value=\"Set Speed\">";
    html += "</form>";
    html += "<form action=\"/stepper/accel\" method=\"POST\">";
    html += "Acceleration: <input name=\"accel\" type=\"number\">";
    html += "<input type=\"submit\" value=\"Set Acceleration\">";
    html += "</form>";
    html += "<form action=\"/stepper/stop\" method=\"POST\">";
    html += "<input type=\"submit\" value=\"Stop Motor\">";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
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

void ServerManager::handleStepperMove(AsyncWebServerRequest *request) {
    if (request->hasParam("position", true)) {
        long position = request->getParam("position", true)->value().toInt();
        stepper.moveTo(position);
        request->send(200, "text/plain", "Moving to position: " + String(position));
    } else {
        request->send(400, "text/plain", "Missing 'position' parameter");
    }
}

void ServerManager::handleStepperStop(AsyncWebServerRequest *request) {
    stepper.stop();
    request->send(200, "text/plain", "Stepper motor stopped");
}

void ServerManager::handleStepperSpeed(AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
        float speed = request->getParam("speed", true)->value().toFloat();
        stepper.setSpeed(speed);
        request->send(200, "text/plain", "Speed set to: " + String(speed));
    } else {
        request->send(400, "text/plain", "Missing 'speed' parameter");
    }
}

void ServerManager::handleStepperAccel(AsyncWebServerRequest *request) {
    if (request->hasParam("accel", true)) {
        float accel = request->getParam("accel", true)->value().toFloat();
        stepper.setAcceleration(accel);
        request->send(200, "text/plain", "Acceleration set to: " + String(accel));
    } else {
        request->send(400, "text/plain", "Missing 'accel' parameter");
    }
} 