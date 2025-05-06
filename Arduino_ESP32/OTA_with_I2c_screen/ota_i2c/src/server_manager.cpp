#include "server_manager.h"
#include <ESP.h>
#include "git_version.h"
#include "config.h"
#include "led_control.h"

ServerManager::ServerManager(DisplayManager& display, StepperManager& stepper) 
    : server(80), ws("/ws"), display(display), stepper(stepper) {}

void ServerManager::init() {
    // Setup WebSocket
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onWebSocketEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);

    // Setup server routes
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleRoot(request); });
    
    server.on("/text", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleText(request); });
    
    server.on("/version", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleVersion(request); });
    
    server.on("/memory", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleMemoryStatus(request); });
    
    server.on("/debug", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleDebug(request); });

    // Stepper motor control endpoints
    server.on("/stepper/move", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperMove(request); });

    server.on("/stepper/stop", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperStop(request); });

    server.on("/stepper/speed", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperSpeed(request); });

    server.on("/stepper/accel", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperAccel(request); });

    server.on("/led/pin", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleLedPinConfig(request); });

    server.on("/led/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleLedTest(request); });

    server.on("/wifi/reset", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleWifiReset(request); });

    server.begin();
    _initialized = true;
}

void ServerManager::handleClient() {
    // Broadcast status updates periodically
    unsigned long currentMillis = millis();
    if (currentMillis - _lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
        broadcastStatus();
        _lastStatusUpdate = currentMillis;
    }
}

void ServerManager::broadcastStatus() {
    if (ws.count() == 0) return; // No clients connected

    StaticJsonDocument<512> doc;
    doc["position"] = stepper.getCurrentPosition();
    doc["speed"] = stepper.getCurrentSpeed();
    doc["acceleration"] = stepper.getCurrentAcceleration();
    doc["isRunning"] = stepper.isRunning();
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["freeHeap"] = ESP.getFreeHeap();

    String jsonString;
    serializeJson(doc, jsonString);
    ws.textAll(jsonString);
}

void ServerManager::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            // Handle incoming WebSocket data if needed
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
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
    String html = generateHeader();
    html += generateTextDisplayForm();
    html += generateStepperControlForms();
    html += generateLedControlForms();
    html += generateWifiControlForm();
    html += generateFooter();
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
    #ifdef FIRMWARE_GIT_COMMIT_HASH
        versionInfo = "Firmware Info:\nCommit: " + String(FIRMWARE_GIT_COMMIT_HASH);
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
        request->redirect("/");
    } else {
        request->send(400, "text/plain", "Missing 'position' parameter");
    }
}

void ServerManager::handleStepperStop(AsyncWebServerRequest *request) {
    stepper.stop();
    request->redirect("/");
}

void ServerManager::handleStepperSpeed(AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
        float speed = request->getParam("speed", true)->value().toFloat();
        stepper.setSpeed(speed);
        request->redirect("/");
    } else {
        request->send(400, "text/plain", "Missing 'speed' parameter");
    }
}

void ServerManager::handleStepperAccel(AsyncWebServerRequest *request) {
    if (request->hasParam("accel", true)) {
        float accel = request->getParam("accel", true)->value().toFloat();
        stepper.setAcceleration(accel);
        request->redirect("/");
    } else {
        request->send(400, "text/plain", "Missing 'accel' parameter");
    }
}

void ServerManager::handleLedPinConfig(AsyncWebServerRequest *request) {
    if (request->hasParam("pin", true)) {
        int newPin = request->getParam("pin", true)->value().toInt();
        if (newPin > 0 && newPin < 40)  // Validate pin number
        {
            LedControl::saveLedPin(newPin);
            request->redirect("/");  // Redirect to home page after successful update
        } else {
            request->send(400, "text/plain", "Invalid pin number. Must be between 1 and 39.");
        }
    } else {
        request->send(400, "text/plain", "Missing 'pin' parameter");
    }
}

void ServerManager::handleLedTest(AsyncWebServerRequest *request) {
    int ledPin = LedControl::getLedPin();
    LedControl led(ledPin);
    led.begin();
    led.blink(3, 200);  // Blink 3 times with 200ms delay
    request->send(200, "text/plain", "LED test completed on pin " + String(ledPin));
}

void ServerManager::handleWifiReset(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Resetting WiFi configuration...");
    delay(1000);  // Give time for response to be sent
    WiFi.disconnect(true);
    ESP.restart();
}

String ServerManager::generateHeader() {
    String html = "<html><head>";
    html += "<title>ESP32 Stepper Motor Control</title>";
    html += "<script>";
    html += "let ws = new WebSocket('ws://' + window.location.hostname + '/ws');";
    html += "ws.onmessage = function(event) {";
    html += "  const data = JSON.parse(event.data);";
    html += "  document.getElementById('position').textContent = data.position;";
    html += "  document.getElementById('speed').textContent = data.speed;";
    html += "  document.getElementById('accel').textContent = data.acceleration;";
    html += "  document.getElementById('status').textContent = data.isRunning ? 'Running' : 'Stopped';";
    html += "  document.getElementById('status').style.color = data.isRunning ? 'blue' : 'green';";
    html += "  document.getElementById('uptime').textContent = data.uptime;";
    html += "  document.getElementById('rssi').textContent = data.rssi;";
    html += "  document.getElementById('heap').textContent = data.freeHeap;";
    html += "};";
    html += "</script>";
    html += "</head><body>";
    html += "<h1>ESP32 Stepper Motor Control</h1>";
    html += "<p>Current Position: <span id='position'>" + String(stepper.getCurrentPosition()) + "</span> steps</p>";
    html += "<p>Microstepping: 1/" + String(stepper.getMicrosteps()) + " (800 steps/rev)</p>";
    html += "<p>Status: <span id='status' style='color: " + String(stepper.isRunning() ? "blue" : "green") + ";'>" + String(stepper.isRunning() ? "Running" : "Stopped") + "</span></p>";
    html += "<p>Uptime: <span id='uptime'>0</span> seconds</p>";
    html += "<p>WiFi Signal: <span id='rssi'>0</span> dBm</p>";
    html += "<p>Free Heap: <span id='heap'>0</span> bytes</p>";
    return html;
}

String ServerManager::generateTextDisplayForm() {
    String html = "<form action=\"/text\" method=\"POST\">";
    html += "Text to display: <input name=\"text\" type=\"text\">";
    html += "<input type=\"submit\" value=\"Display\">";
    html += "</form>";
    return html;
}

String ServerManager::generateStepperControlForms() {
    String html = "<h2>Stepper Motor Control</h2>";
    
    // Move form
    html += "<form action=\"/stepper/move\" method=\"POST\">";
    html += "Target Position: <input name=\"position\" type=\"number\" value=\"" + String(stepper.getCurrentPosition()) + "\">";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('position')[0].value = parseInt(document.getElementsByName('position')[0].value) + 800\">+1 Rev</button>";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('position')[0].value = parseInt(document.getElementsByName('position')[0].value) - 800\">-1 Rev</button>";
    html += "<input type=\"submit\" value=\"Move\">";
    html += "</form>";

    // Speed form
    html += "<form action=\"/stepper/speed\" method=\"POST\">";
    html += "Speed (steps/sec): <input name=\"speed\" type=\"number\" value=\"" + String(stepper.getCurrentSpeed()) + "\">";
    html += "<input type=\"submit\" value=\"Set Speed\">";
    html += "</form>";

    // Acceleration form
    html += "<form action=\"/stepper/accel\" method=\"POST\">";
    html += "Acceleration (steps/sec²): <input name=\"accel\" type=\"number\" value=\"" + String(stepper.getCurrentAcceleration()) + "\">";
    html += "<input type=\"submit\" value=\"Set Acceleration\">";
    html += "</form>";

    // Stop form
    html += "<form action=\"/stepper/stop\" method=\"POST\">";
    html += "<input type=\"submit\" value=\"Stop Motor\">";
    html += "</form>";

    return html;
}

String ServerManager::generateLedControlForms() {
    String html = "<h2>LED Configuration</h2>";
    html += "<form action=\"/led/pin\" method=\"POST\">";
    html += "LED Pin (1-39): <input name=\"pin\" type=\"number\" min=\"1\" max=\"39\" value=\"" + String(LedControl::getLedPin()) + "\">";
    html += "<input type=\"submit\" value=\"Update LED Pin\">";
    html += "</form>";
    
    html += "<h2>LED Test</h2>";
    html += "<form action=\"/led/test\" method=\"GET\">";
    html += "<input type=\"submit\" value=\"Test LED\">";
    html += "</form>";
    
    return html;
}

String ServerManager::generateWifiControlForm() {
    String html = "<h2>WiFi Configuration</h2>";
    html += "<form action=\"/wifi/reset\" method=\"GET\">";
    html += "<input type=\"submit\" value=\"Reset WiFi\" style=\"color: red;\">";
    html += "</form>";
    return html;
}

String ServerManager::generateFooter() {
    return "</body></html>";
} 