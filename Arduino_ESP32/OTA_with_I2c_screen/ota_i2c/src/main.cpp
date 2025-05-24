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
#include "control_signal_handler.h"
#include "flash_controller.h"
#include "my_wifi_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
PinManager pinManager(display);
StepperManager stepperMotor(display);
ServerManager serverManager(display, stepperMotor, pinManager);
OTAManager otaManager(display);
LedControl led;  // Fixed LED initialization
ControlSignalHandler signalHandler(display);

void setup() 
{
  delay(1000);  // Give the system time to stabilize
  Serial.begin(115200);
  while (!Serial) {  // Wait for Serial to be ready
    delay(10);
  }
  
  // Initialize Flash controller first
  Serial.println("Initializing Flash controller...");
  if (!FlashController::init()) {
    Serial.println("Flash initialization failed!");
    ESP.restart();
  }
  Serial.println("Flash controller initialized");
  
  // Basic Serial test
  Serial.print("TEST1\r\n");
  Serial.flush();
  delay(100);
  
  Serial.print("TEST2\r\n");
  Serial.flush();
  delay(100);
  
  Serial.printf("LED Pin: %d\r\n", LedControl::getLedPin());
  Serial.flush();
  delay(100);
  

  // Now proceed with initialization
  led.init();
  Serial.print("LED_OK\r\n");
  Serial.flush();
  delay(50);
  
  if (!display.init()) {
    Serial.print("DISP_ERR\r\n");
    Serial.flush();
    ESP.restart();
  }
  Serial.print("DISP_OK\r\n");
  Serial.flush();
  delay(50);

  if (!pinManager.init()) {
    Serial.print("PIN_ERR\r\n");
    Serial.flush();
    onFailure("Pin Manager Init Failed", display, led);
  }
  Serial.print("PIN_OK\r\n");
  Serial.flush();
  delay(50);

  if (!stepperMotor.init()) {
    Serial.print("STEP_ERR\r\n");
    Serial.flush();
    onFailure("Stepper Init Failed", display, led);
  }
  Serial.print("STEP_OK\r\n");
  Serial.flush();
  delay(50);

  if (!signalHandler.init()) {
    Serial.print("SIG_ERR\r\n");
    Serial.flush();
    onFailure("Signal Handler Init Failed", display, led);
  }
  Serial.print("SIG_OK\r\n");
  Serial.flush();
  delay(50);
  
  Serial.print("ADD_SW\r\n");
  Serial.flush();
  delay(50);
  
  // Add limit switches with activeLow = false since they are active-high
  // Using GPIO18 and GPIO19 which are general purpose I/O pins
  if (!signalHandler.addLimitSwitch(18, "HOME_SWITCH", false)) {
    Serial.print("SW1_ERR\r\n");
    Serial.flush();
  } else {
    Serial.print("SW1_OK\r\n");
    Serial.flush();
  }
  delay(50);
  
  if (!signalHandler.addLimitSwitch(19, "END_SWITCH", false)) {
    Serial.print("SW2_ERR\r\n");
    Serial.flush();
  } else {
    Serial.print("SW2_OK\r\n");
    Serial.flush();
  }
  delay(50);
  
  Serial.print("SW_OK\r\n");
  Serial.flush();
  delay(50);

  // Register signal handler
  signalHandler.registerSignalHandler([](const char* switchId) {
    Serial.printf("SW_ACT:%s\r\n", switchId);
    Serial.flush();
    display.displayLines({
      "Limit Switch",
      switchId,
      "Activated!"
    });
  });
  Serial.print("HAND_OK\r\n");
  Serial.flush();
  delay(50);

  display.displayLines({"WiFi Setup Mode", "Connect to:", "ESP32-Setup"});

  if (!MyWiFiManager::instance().autoConnect("ESP32-Setup")) {
    Serial.print("WIFI_ERR\r\n");
    Serial.flush();
    onFailure("WiFi Connect Failed", display, led);
  }
  Serial.print("WIFI_OK\r\n");
  Serial.flush();
  delay(50);

  if (!serverManager.init()) {
    Serial.print("SRV_ERR\r\n");
    Serial.flush();
    onFailure("Server Init Failed", display, led);
  }
  Serial.print("SRV_OK\r\n");
  Serial.flush();
  delay(50);
  
  display.displayLines({"HTTP server started", WiFi.localIP().toString()});

  if (!otaManager.init("esp32-blinker", "haslo123")) {
    Serial.print("OTA_ERR\r\n");
    Serial.flush();
    onFailure("OTA Init Failed", display, led);
  }
  Serial.print("OTA_OK\r\n");
  Serial.flush();
  delay(50);

  displayFinalConnectionInfo(display);
  Serial.print("DONE\r\n");
  Serial.flush();
}

void loop() 
{
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {  // Print every second
    Serial.print("LOOP\r\n");
    lastPrint = millis();
  }
  
  otaManager.handle();
  serverManager.handleClient();
  stepperMotor.run();
  signalHandler.handle();
} 