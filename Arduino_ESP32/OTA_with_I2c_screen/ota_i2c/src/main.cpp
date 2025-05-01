#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "ota_manager.h"
#include "display_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int ledPin = 12;  // GPIO12
DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
AsyncWebServer server(80);
OTAManager otaManager(display);

// Text display handler
void handleText(AsyncWebServerRequest *request) {
  if (request->hasParam("text", true)) {
    String text = request->getParam("text", true)->value();
    display.displayText(text.c_str());
    request->send(200, "text/plain", "Text displayed: " + text);
  } else {
    request->send(400, "text/plain", "Missing 'text' parameter");
  }
}

// Root page with form
void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html",
                "<form action=\"/text\" method=\"POST\">"
                "Text to display: <input name=\"text\" type=\"text\">"
                "<input type=\"submit\" value=\"Display\">"
                "</form>");
}

// Memory status handler
void handleMemoryStatus(AsyncWebServerRequest *request) {
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

// Version info handler
void handleVersion(AsyncWebServerRequest *request) {
  String versionInfo;
  #ifdef GIT_COMMIT_HASH
    versionInfo = "Firmware Info:\nCommit: " + String(GIT_COMMIT_HASH);
  #else
    versionInfo = "Firmware Info:\nCommit: unknown";
  #endif
  request->send(200, "text/plain", versionInfo);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Initialize OLED display
  if (!display.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  WiFiManager wm;
  display.displayLines({"WiFi Setup Mode", 
               "Connect to:", 
               "ESP32-Setup"});
  bool ok = wm.autoConnect("ESP32-Setup");

  if (!ok) {
    display.displayText("WiFi Connect Failed");
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Async server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/text", HTTP_POST, handleText);
  server.on("/version", HTTP_GET, handleVersion);
  
  // Add a debug endpoint
  server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {
    String debugInfo = "Debug Info:\n";
    debugInfo += "WiFi Status: " + String(WiFi.status()) + "\n";
    debugInfo += "IP Address: " + WiFi.localIP().toString() + "\n";
    debugInfo += "MAC Address: " + WiFi.macAddress() + "\n";
    debugInfo += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    request->send(200, "text/plain", debugInfo);
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Server running on IP: ");
  Serial.println(WiFi.localIP());

  // Initialize OTA
  otaManager.begin("esp32-blinker", "haslo123");
  Serial.println("OTA Ready");

  // Display final connection info
  String ip = WiFi.localIP().toString();
  #ifdef GIT_COMMIT_HASH
    std::vector<String> lines = {
        "WiFi Connected!",
        ip,
        "OTA: esp32-blinker",
        "Commit: " + String(GIT_COMMIT_HASH)
    };
    display.displayLines(lines);
  #else
    std::vector<String> lines = {
        "WiFi Connected!",
        ip,
        "OTA: esp32-blinker"
    };
    display.displayLines(lines);
  #endif
}

void loop() {
  otaManager.handle();
} 