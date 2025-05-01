#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>
#include <Adafruit_SSD1306.h>

class OTAManager {
public:
    OTAManager(Adafruit_SSD1306& display);
    void begin(const char* hostname, const char* password);
    void handle();

private:
    Adafruit_SSD1306& _display;
    
    void onStart();
    void onProgress(unsigned int progress, unsigned int total);
    void onEnd();
    void onError(ota_error_t error);
};

#endif // OTA_MANAGER_H 