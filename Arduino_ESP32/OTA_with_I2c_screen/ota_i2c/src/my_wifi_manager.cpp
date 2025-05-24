#include "my_wifi_manager.h"
#include <WiFiManager.h>

void MyWiFiManager::resetSettings() {
    WiFiManager wm;
    wm.resetSettings();
} 