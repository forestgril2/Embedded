#ifndef MAIN_SETUP_HELPERS_H
#define MAIN_SETUP_HELPERS_H

#include <Arduino.h>
#include <WiFi.h>
#include "display_manager.h"
#include "led_control.h"
#include "git_version.h"

// Helper function to handle initialization failures
void onFailure(const String&& message, DisplayManager& display, LedControl& led) {
    display.displayText(message.c_str());
    led.blink(5, 500);
    ESP.restart();
}

// Helper function to display final connection information
void displayFinalConnectionInfo(DisplayManager& display) {
    String ip = WiFi.localIP().toString();
    #ifdef FIRMWARE_GIT_COMMIT_HASH
        display.displayLines({"WiFi Connected!",
                            ip,
                            "OTA: esp32-servo-tester",
                            "Hash: " + String(FIRMWARE_GIT_COMMIT_HASH)});
    #else
        display.displayLines({"WiFi Connected!",
                            ip,
                            "OTA: esp32-servo-tester"});
    #endif
    Serial.println("WiFi connection ready");
}

#endif // MAIN_SETUP_HELPERS_H 