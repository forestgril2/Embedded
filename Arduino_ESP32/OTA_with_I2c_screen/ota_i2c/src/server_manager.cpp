#include "server_manager.h"
#include <ESP.h>
#include "git_version.h"
#include "config.h"
#include "led_control.h"
#include "memory_manager.h"
#include "pin_manager.h"
#include "my_wifi_manager.h"

ServerManager::ServerManager(DisplayManager& display, StepperManager& stepper, PinManager& pinManager) 
    : server(80), ws("/ws"), display(display), stepper(stepper), pinManager(pinManager) {}

bool ServerManager::init() {
    try {
        // Setup WebSocket
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
            this->onWebSocketEvent(server, client, type, arg, data, len);
        });
        server.addHandler(&ws);

        // Setup server routes
        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleRoot(request); });
        server.on("/led", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleLedPage(request); });
        server.on("/pins", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePinPage(request); });
        server.on("/system", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleSystemPage(request); });
        
        // API endpoints
        server.on("/text", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleText(request); });
        server.on("/version", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleVersion(request); });
        server.on("/memory", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleMemoryStatus(request); });
        server.on("/debug", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleDebug(request); });

        // Stepper motor control endpoints
        server.on("/stepper/move", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperMove(request); });
        server.on("/stepper/stop", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperStop(request); });
        server.on("/stepper/speed", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperSpeed(request); });
        server.on("/stepper/accel", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperAccel(request); });
        server.on("/stepper/torque", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleStepperTorque(request); });

        // LED control endpoints
        server.on("/led/pin", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleLedPinConfig(request); });
        server.on("/led/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleLedTest(request); });

        // Pin configuration endpoints
        server.on("/pins/config", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handlePinConfig(request); });
        server.on("/pins/config", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePinConfigGet(request); });

        // System endpoints
        server.on("/system/wifi/reset", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleWifiReset(request); });

        server.begin();
        _initialized = true;
        return true;
    } catch (...) {
        _initialized = false;
        return false;
    }
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
    doc["holdingTorque"] = stepper.isHoldingTorqueEnabled();
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
    html += "<h1>Stepper Motor Control</h1>";
    html += generateStepperControlForms();
    html += generateFooter();
    request->send(200, "text/html", html);
}

void ServerManager::handleLedPage(AsyncWebServerRequest *request) {
    String html = generateHeader();
    html += "<h1>LED Control</h1>";
    html += generateLedControlForms();
    html += generateFooter();
    request->send(200, "text/html", html);
}

void ServerManager::handlePinPage(AsyncWebServerRequest *request) {
    String html = generateHeader();
    html += "<h1>Pin Configuration</h1>";
    html += generatePinConfigForm();
    html += generateFooter();
    request->send(200, "text/html", html);
}

void ServerManager::handleSystemPage(AsyncWebServerRequest *request) {
    String html = generateHeader();
    html += "<h1>System Information</h1>";
    html += "<div class='form-group'>";
    html += "<h2>System Status</h2>";
    html += "<p>Uptime: <span id='uptime'>0</span> seconds</p>";
    html += "<p>WiFi Signal: <span id='rssi'>0</span> dBm</p>";
    html += "<p>Free Heap: <span id='heap'>0</span> bytes</p>";
    html += "</div>";
    html += generateWifiControlForm();
    html += generateFooter();
    request->send(200, "text/html", html);
}

void ServerManager::handleMemoryStatus(AsyncWebServerRequest *request) {
    request->send(200, "application/json", MemoryManager::getStatusJson());
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

void ServerManager::sendJsonResponse(AsyncWebServerRequest *request, int code, bool success, const char* error) {
    StaticJsonDocument<128> doc;
    doc["success"] = success;
    if (error) {
        doc["error"] = error;
    }
    String response;
    serializeJson(doc, response);
    request->send(code, "application/json", response);
}

template<typename T>
void ServerManager::sendJsonResponse(AsyncWebServerRequest *request, int code, bool success, const char* key, T value, const char* error) {
    StaticJsonDocument<128> doc;
    doc["success"] = success;
    doc[key] = value;
    if (error) {
        doc["error"] = error;
    }
    String response;
    serializeJson(doc, response);
    request->send(code, "application/json", response);
}

void ServerManager::handleStepperMove(AsyncWebServerRequest *request) {
    if (request->hasParam("position", true)) {
        long position = request->getParam("position", true)->value().toInt();
        _targetPosition = position;  // Store the target position
        stepper.moveTo(position);
        sendJsonResponse(request, 200, true, "targetPosition", position);
    } else {
        sendJsonResponse(request, 400, false, "Missing position parameter");
    }
}

void ServerManager::handleStepperStop(AsyncWebServerRequest *request) {
    stepper.stop();
    sendJsonResponse(request, 200, true);
}

void ServerManager::handleStepperSpeed(AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
        float speed = request->getParam("speed", true)->value().toFloat();
        stepper.setSpeed(speed);
        sendJsonResponse(request, 200, true, "speed", speed);
    } else {
        sendJsonResponse(request, 400, false, "Missing speed parameter");
    }
}

void ServerManager::handleStepperAccel(AsyncWebServerRequest *request) {
    if (request->hasParam("accel", true)) {
        float accel = request->getParam("accel", true)->value().toFloat();
        stepper.setAcceleration(accel);
        sendJsonResponse(request, 200, true, "accel", accel);
    } else {
        sendJsonResponse(request, 400, false, "Missing accel parameter");
    }
}

void ServerManager::handleStepperTorque(AsyncWebServerRequest *request) {
    if (request->hasParam("enable", true)) {
        bool enable = request->getParam("enable", true)->value() == "true";
        stepper.setHoldingTorque(enable);
    }
    request->redirect("/");  // Always redirect back to main page
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
    led.init();
    led.blink(3, 200);  // Blink 3 times with 200ms delay
    request->send(200, "text/plain", "LED test completed on pin " + String(ledPin));
}

void ServerManager::handleWifiReset(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Resetting WiFi configuration...");
    delay(1000);  // Give time for response to be sent
    MyWiFiManager::resetSettings();
    ESP.restart();
}

void ServerManager::handlePinConfig(AsyncWebServerRequest *request) {
    if (!request->hasParam("stepperStepPin", true) ||
        !request->hasParam("stepperDirPin", true) ||
        !request->hasParam("stepperEnablePin", true) ||
        !request->hasParam("displaySdaPin", true) ||
        !request->hasParam("displaySclPin", true) ||
        !request->hasParam("displayResetPin", true) ||
        !request->hasParam("ledPin", true)) {
        request->send(400, "text/plain", "Missing parameters");
        return;
    }

    PinManager::PinConfig config;
    config.stepperStepPin = request->getParam("stepperStepPin", true)->value().toInt();
    config.stepperDirPin = request->getParam("stepperDirPin", true)->value().toInt();
    config.stepperEnablePin = request->getParam("stepperEnablePin", true)->value().toInt();
    config.displaySdaPin = request->getParam("displaySdaPin", true)->value().toInt();
    config.displaySclPin = request->getParam("displaySclPin", true)->value().toInt();
    config.displayResetPin = request->getParam("displayResetPin", true)->value().toInt();
    config.ledPin = request->getParam("ledPin", true)->value().toInt();

    // Validate pins
    if (!PinManager::validatePin(config.stepperStepPin) ||
        !PinManager::validatePin(config.stepperDirPin) ||
        !PinManager::validatePin(config.stepperEnablePin) ||
        !PinManager::validatePin(config.displaySdaPin) ||
        !PinManager::validatePin(config.displaySclPin) ||
        !PinManager::validatePin(config.displayResetPin) ||
        !PinManager::validatePin(config.ledPin)) {
        request->send(400, "text/plain", "Invalid pin numbers");
        return;
    }

    pinManager.saveConfig(config);
    request->send(200, "text/plain", "Configuration saved");
}

void ServerManager::handlePinConfigGet(AsyncWebServerRequest *request) {
    request->send(200, "application/json", pinManager.getConfigJson());
}

String ServerManager::generateHeader() {
    String html = "<html><head>";
    html += "<title>ESP32 Stepper Motor Control</title>";
    html += "<style>";
    html += "  body { font-family: Arial, sans-serif; margin: 20px; }";
    html += "  .nav { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
    html += "  .nav a { margin-right: 15px; text-decoration: none; color: #333; }";
    html += "  .nav a:hover { color: #0066cc; }";
    html += "  .status-value { font-weight: bold; }";
    html += "  .speed-value { color: #0066cc; }";
    html += "  .status-running { color: blue; }";
    html += "  .status-stopped { color: green; }";
    html += "  .form-group { margin-bottom: 15px; }";
    html += "  .form-group label { display: block; margin-bottom: 5px; }";
    html += "  .form-group input[type='number'] { width: 100px; }";
    html += "</style>";
    html += "<script>";
    html += "let ws = new WebSocket('ws://' + window.location.hostname + '/ws');";
    html += "ws.onmessage = function(event) {";
    html += "  const data = JSON.parse(event.data);";
    html += "  if (document.getElementById('current-position')) {";
    html += "    document.getElementById('current-position').textContent = data.position;";
    html += "    document.getElementById('speed').textContent = data.speed.toFixed(1);";
    html += "    document.getElementById('accel').textContent = data.acceleration.toFixed(1);";
    html += "    const statusElem = document.getElementById('status');";
    html += "    statusElem.textContent = data.isRunning ? 'Running' : 'Stopped';";
    html += "    statusElem.className = data.isRunning ? 'status-running' : 'status-stopped';";
    html += "  }";
    html += "  if (document.getElementById('uptime')) {";
    html += "    document.getElementById('uptime').textContent = data.uptime;";
    html += "    document.getElementById('rssi').textContent = data.rssi;";
    html += "    document.getElementById('heap').textContent = data.freeHeap;";
    html += "  }";
    html += "};";
    
    // Add form submission handling
    html += "function submitForm(form) {";
    html += "  event.preventDefault();";
    html += "  fetch(form.action, {";
    html += "    method: form.method,";
    html += "    body: new FormData(form)";
    html += "  })";
    html += "  .then(response => response.json())";
    html += "  .then(data => {";
    html += "    if (data.success) {";
    html += "      if (data.targetPosition !== undefined) {";
    html += "        document.getElementsByName('position')[0].value = data.targetPosition;";
    html += "      }";
    html += "      if (data.speed !== undefined) {";
    html += "        document.getElementsByName('speed')[0].value = data.speed;";
    html += "      }";
    html += "      if (data.accel !== undefined) {";
    html += "        document.getElementsByName('accel')[0].value = data.accel;";
    html += "      }";
    html += "    }";
    html += "  });";
    html += "  return false;";
    html += "}";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='nav'>";
    html += "<a href='/'>Stepper Control</a>";
    html += "<a href='/led'>LED Control</a>";
    html += "<a href='/pins'>Pin Configuration</a>";
    html += "<a href='/system'>System Info</a>";
    html += "</div>";
    return html;
}

String ServerManager::generateStepperControlForms() {
    String html = "<h2>Stepper Motor Control</h2>";
    
    // Status display
    html += "<div class='form-group'>";
    html += "<h3>Current Status</h3>";
    html += "<p>Position: <span id='current-position' class='status-value'>" + String(stepper.getCurrentPosition()) + "</span> steps</p>";
    html += "<p>Current Speed: <span id='speed' class='speed-value'>" + String(stepper.getCurrentSpeed(), 1) + "</span> steps/sec</p>";
    html += "<p>Acceleration: <span id='accel' class='status-value'>" + String(stepper.getCurrentAcceleration(), 1) + "</span> steps/sec²</p>";
    html += "<p>Microstepping: 1/" + String(stepper.getMicrosteps()) + " (800 steps/rev)</p>";
    html += "<p>Status: <span id='status' class='" + String(stepper.isRunning() ? "status-running" : "status-stopped") + "'>" + String(stepper.isRunning() ? "Running" : "Stopped") + "</span></p>";
    html += "</div>";
    
    // Move form
    html += "<form action=\"/stepper/move\" method=\"POST\" onsubmit=\"return submitForm(this);\">";
    html += "Target Position: <input name=\"position\" type=\"number\" value=\"" + String(stepper.getCurrentPosition()) + "\">";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('position')[0].value = parseInt(document.getElementsByName('position')[0].value) + 800\">+1 Rev</button>";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('position')[0].value = parseInt(document.getElementsByName('position')[0].value) - 800\">-1 Rev</button>";
    html += "<input type=\"submit\" value=\"Move\">";
    html += "</form>";

    // Speed form
    html += "<form action=\"/stepper/speed\" method=\"POST\" onsubmit=\"return submitForm(this);\">";
    html += "Speed (steps/sec): <input name=\"speed\" type=\"number\" value=\"" + String(stepper.getCurrentSpeed()) + "\">";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('speed')[0].value = parseInt(document.getElementsByName('speed')[0].value) + 400\">+400</button>";
    html += "<button type=\"button\" onclick=\"document.getElementsByName('speed')[0].value = parseInt(document.getElementsByName('speed')[0].value) - 400\">-400</button>";
    html += "<input type=\"submit\" value=\"Set Speed\">";
    html += "</form>";

    // Acceleration form
    html += "<form action=\"/stepper/accel\" method=\"POST\" onsubmit=\"return submitForm(this);\">";
    html += "Acceleration (steps/sec²): <input name=\"accel\" type=\"number\" value=\"" + String(stepper.getCurrentAcceleration()) + "\">";
    html += "<input type=\"submit\" value=\"Set Acceleration\">";
    html += "</form>";

    // Stop form
    html += "<form action=\"/stepper/stop\" method=\"POST\" onsubmit=\"return submitForm(this);\">";
    html += "<input type=\"submit\" value=\"Stop Motor\">";
    html += "</form>";

    // Holding Torque form
    html += "<form action=\"/stepper/torque\" method=\"POST\">";
    html += "<label>Holding Torque: <input type=\"checkbox\" name=\"enable\" value=\"true\" " + String(stepper.isHoldingTorqueEnabled() ? "checked" : "") + " onchange=\"this.form.submit()\">";
    html += "<input type=\"hidden\" name=\"enable\" value=\"false\">";  // This will be sent when checkbox is unchecked
    html += "</label></form>";

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

String ServerManager::generatePinConfigForm() {
    PinManager::PinConfig config = pinManager.loadConfig();
    String html = "<h2>Pin Configuration</h2>";
    html += "<form action='/pin-config' method='POST'>";
    html += "<label>Stepper Step Pin: <input type='number' name='stepperStepPin' value='" + String(config.stepperStepPin) + "'></label><br>";
    html += "<label>Stepper Direction Pin: <input type='number' name='stepperDirPin' value='" + String(config.stepperDirPin) + "'></label><br>";
    html += "<label>Stepper Enable Pin: <input type='number' name='stepperEnablePin' value='" + String(config.stepperEnablePin) + "'></label><br>";
    html += "<label>Display SDA Pin: <input type='number' name='displaySdaPin' value='" + String(config.displaySdaPin) + "'></label><br>";
    html += "<label>Display SCL Pin: <input type='number' name='displaySclPin' value='" + String(config.displaySclPin) + "'></label><br>";
    html += "<label>Display Reset Pin: <input type='number' name='displayResetPin' value='" + String(config.displayResetPin) + "'></label><br>";
    html += "<label>LED Pin: <input type='number' name='ledPin' value='" + String(config.ledPin) + "'></label><br>";
    html += "<input type='submit' value='Save Configuration'>";
    html += "</form>";
    return html;
}

String ServerManager::generateWifiControlForm() {
    String html = "<h2>WiFi Configuration</h2>";
    html += "<form action=\"/system/wifi/reset\" method=\"GET\">";
    html += "<input type=\"submit\" value=\"Reset WiFi\" style=\"color: red;\">";
    html += "</form>";
    return html;
}

String ServerManager::generateFooter() {
    return "</body></html>";
} 