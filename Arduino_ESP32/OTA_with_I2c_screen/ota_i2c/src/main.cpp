#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include "ota_manager.h"
#include "display_manager.h"
#include "server_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int ledPin = 12;  // GPIO12
DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
ServerManager serverManager(display);
OTAManager otaManager(display);

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

  // Initialize server
  serverManager.begin();
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
  serverManager.handleClient();
} 