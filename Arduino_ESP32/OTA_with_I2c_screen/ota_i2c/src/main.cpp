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
#include "pin_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
PinManager pinManager(display);
StepperManager stepperMotor(display);
ServerManager serverManager(display, stepperMotor, pinManager);
OTAManager otaManager(display);
LedControl led(LedControl::getLedPin());

void onFailure(const String&& message)
{
  display.displayText(message.c_str());
  led.blink(5, 500);
  ESP.restart();
}

void displayFinalConnectionInfo()
{
  String ip = WiFi.localIP().toString();
  #ifdef FIRMWARE_GIT_COMMIT_HASH
    display.displayLines({"WiFi Connected!",
                          ip,
                          "OTA: esp32-blinker",
                          "Hash: " + String(FIRMWARE_GIT_COMMIT_HASH)});
  #else
    display.displayLines({"WiFi Connected!",
                          ip,
                          "OTA: esp32-blinker"});
  #endif
}

void setup() 
{
  Serial.begin(115200);
  led.begin();
  
  if (!display.init())
    ESP.restart();

  if (!pinManager.init())
    onFailure("Pin Manager Init Failed");

  if (!stepperMotor.init())
    onFailure("Stepper Init Failed");

  WiFiManager wm;
  display.displayLines({"WiFi Setup Mode", "Connect to:", "ESP32-Setup"});

  if (!wm.autoConnect("ESP32-Setup")) 
    onFailure("WiFi Connect Failed");

  if (!serverManager.init())
    onFailure("Server Init Failed");
  
  display.displayLines({"HTTP server started", WiFi.localIP().toString()});

  if (!otaManager.init("esp32-blinker", "haslo123"))
    onFailure("OTA Init Failed"); 
  Serial.println("OTA Ready");

  displayFinalConnectionInfo();
}

void loop() 
{
  otaManager.handle();
  serverManager.handleClient();
  stepperMotor.run();
} 