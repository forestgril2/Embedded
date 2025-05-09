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
#include "main_setup_helpers.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
PinManager pinManager(display);
StepperManager stepperMotor(display);
ServerManager serverManager(display, stepperMotor, pinManager);
OTAManager otaManager(display);
LedControl led(LedControl::getLedPin());

void setup() 
{
  Serial.begin(115200);
  led.begin();
  
  if (!display.init())
    ESP.restart();

  if (!pinManager.init())
    onFailure("Pin Manager Init Failed", display, led);

  if (!stepperMotor.init())
    onFailure("Stepper Init Failed", display, led);

  WiFiManager wm;
  display.displayLines({"WiFi Setup Mode", "Connect to:", "ESP32-Setup"});

  if (!wm.autoConnect("ESP32-Setup")) 
    onFailure("WiFi Connect Failed", display, led);

  if (!serverManager.init())
    onFailure("Server Init Failed", display, led);
  
  display.displayLines({"HTTP server started", WiFi.localIP().toString()});

  if (!otaManager.init("esp32-blinker", "haslo123"))
    onFailure("OTA Init Failed", display, led); 
  Serial.println("OTA Ready");

  displayFinalConnectionInfo(display);
}

void loop() 
{
  otaManager.handle();
  serverManager.handleClient();
  stepperMotor.run();
} 