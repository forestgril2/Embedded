#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "ota_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int ledPin = 12;  // GPIO12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
OTAManager otaManager(display);

// Function to display text on OLED
void displayText(const char* text, int line = 0) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, line * 10);
  display.setTextColor(SSD1306_WHITE);
  display.println(text);
  display.display();
}

// Function to display multiple lines
void displayLines(const char* line1, const char* line2 = "", const char* line3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.println(line1);
  display.setCursor(0, 10);
  display.println(line2);
  display.setCursor(0, 20);
  display.println(line3);
  display.display();
}

// Function to display memory information
void displayMemoryInfo() {
  char memInfo[64];
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t freePsram = ESP.getFreePsram();
  uint32_t totalPsram = ESP.getPsramSize();
  uint32_t freeSketchSpace = ESP.getFreeSketchSpace();
  uint32_t sketchSize = ESP.getSketchSize();
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  
  sprintf(memInfo, "Heap: %u/%u", freeHeap, totalHeap);
  display.println(memInfo);
  
  if (totalPsram > 0) {
    sprintf(memInfo, "PSRAM: %u/%u", freePsram, totalPsram);
    display.println(memInfo);
  }
  
  sprintf(memInfo, "Flash: %u/%u", freeSketchSpace, sketchSize);
  display.println(memInfo);
  
  display.display();
}

// Text display handler
void handleText(AsyncWebServerRequest *request) {
  if (request->hasParam("text", true)) {
    String text = request->getParam("text", true)->value();
    displayText(text.c_str());
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
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.display();

  WiFiManager wm;
  displayLines("WiFi Setup Mode", 
               "Connect to:", 
               "ESP32-Setup");
  bool ok = wm.autoConnect("ESP32-Setup");

  if (!ok) {
    displayText("WiFi Connect Failed");
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
  displayLines("WiFi Connected!", 
               ip.c_str(),
               "OTA: esp32-blinker");
  Serial.print("Connected! IP: ");
  Serial.println(ip);
}

void loop() {
  otaManager.handle();
} 