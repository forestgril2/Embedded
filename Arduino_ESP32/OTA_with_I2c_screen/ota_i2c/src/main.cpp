#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include "ota_manager.h"
#include "display_manager.h"
#include "server_manager.h"
#include "stepper_manager.h"
#include "led_control.h"
#include "git_version.h"
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
StepperManager stepper(display);
ServerManager serverManager(display, stepper);
OTAManager otaManager(display);
LedControl led(LedControl::getLedPin());

void setup() 
{
  Serial.begin(115200);
  led.begin();
  
  // Initialize OLED display
  display.init();
  if (!display.isInitialized()) 
  {
    ESP.restart();
  }

  // Initialize stepper motor
  stepper.begin();

  WiFiManager wm;
  std::vector<String> setupLines = {
    "WiFi Setup Mode", 
    "Connect to:", 
    "ESP32-Setup"
  };
  display.displayLines(setupLines);
  
  bool ok = wm.autoConnect("ESP32-Setup");

  if (!ok) 
  {
    display.displayText("WiFi Connect Failed");
    Serial.println("Failed to connect. Restarting...");
    led.blink(5, 500);
    delay(3000);
    ESP.restart();
  }

  // Initialize server
  serverManager.init();
  if (!serverManager.isInitialized()) 
  {
    display.displayText("Server Init Failed");
    led.blink(5, 500);
    ESP.restart();
  }
  
  std::vector<String> serverLines = {
    "HTTP server started",
    WiFi.localIP().toString()
  };
  display.displayLines(serverLines);

  // Initialize OTA
  otaManager.init("esp32-blinker", "haslo123");
  if (!otaManager.isInitialized()) 
  {
    display.displayText("OTA Init Failed");
    led.blink(5, 500);
    ESP.restart();
  }
  Serial.println("OTA Ready");

  // Display final connection info
  String ip = WiFi.localIP().toString();
  #ifdef FIRMWARE_GIT_COMMIT_HASH
    std::vector<String> lines = {
        "WiFi Connected!",
        ip,
        "OTA: esp32-blinker",
        "Commit: " + String(FIRMWARE_GIT_COMMIT_HASH)
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

void loop() 
{
  otaManager.handle();
  serverManager.handleClient();
  stepper.run();  // Update stepper motor position
} 