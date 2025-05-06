#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>
#include "display_manager.h"

class OTAManager 
{
public:
    OTAManager(DisplayManager& display);
    void begin(const char* hostname, const char* password);
    bool isInitialized() const { return _initialized; }
    void handle();

private:
    DisplayManager& _display;
    bool _initialized = false;
    
    void onStart();
    void onProgress(unsigned int progress, unsigned int total);
    void onEnd();
    void onError(ota_error_t error);
};

#endif // OTA_MANAGER_H 