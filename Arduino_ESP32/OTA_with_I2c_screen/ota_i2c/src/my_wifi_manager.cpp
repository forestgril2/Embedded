#include "my_wifi_manager.h"
#include <WiFiManager.h>

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
    pImpl->wm.resetSettings();
}

bool MyWiFiManager::autoConnect(const char* apName, const char* apPassword) {
    if (apPassword) {
        return pImpl->wm.autoConnect(apName, apPassword);
    } else {
        return pImpl->wm.autoConnect(apName);
    }
} 