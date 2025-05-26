#include "my_wifi_manager.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <SPIFFS.h>

class MyWiFiManager::Impl {
public:
    WiFiManager wm;
};

MyWiFiManager::MyWiFiManager() : pImpl(new Impl) {}

MyWiFiManager& MyWiFiManager::instance() {
    static MyWiFiManager instance;
    return instance;
}

void MyWiFiManager::resetSettings() {
    // Disconnect from current WiFi
    WiFi.disconnect(true);
    delay(1000);
    
    // Erase WiFi credentials
    WiFi.begin("0", "0");
    delay(1000);
    WiFi.disconnect(true);
    delay(1000);
    
    // Reset WiFiManager settings
    pImpl->wm.resetSettings();
    
    // Force AP mode
    WiFi.mode(WIFI_AP);
    delay(1000);
}

bool MyWiFiManager::autoConnect(const char* apName, const char* apPassword) {
    // Force AP mode first
    WiFi.mode(WIFI_AP);
    delay(1000);
    
    if (apPassword) {
        return pImpl->wm.autoConnect(apName, apPassword);
    } else {
        return pImpl->wm.autoConnect(apName);
    }
} 