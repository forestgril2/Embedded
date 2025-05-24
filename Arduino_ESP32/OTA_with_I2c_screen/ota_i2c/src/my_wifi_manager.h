#ifndef MY_WIFI_MANAGER_H
#define MY_WIFI_MANAGER_H

#include <string>

class MyWiFiManager {
public:
    static MyWiFiManager& instance();
    void resetSettings();
    // Expose autoConnect, returns true on success, false on failure
    bool autoConnect(const char* apName, const char* apPassword = nullptr);
private:
    MyWiFiManager();
    MyWiFiManager(const MyWiFiManager&) = delete;
    MyWiFiManager& operator=(const MyWiFiManager&) = delete;
    class Impl;
    Impl* pImpl;
};

#endif // MY_WIFI_MANAGER_H 