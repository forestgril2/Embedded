#include "ota_manager.h"
#include <ArduinoOTA.h>

OTAManager::OTAManager(DisplayManager& display) : _display(display) 
{
}

void OTAManager::begin(const char* hostname, const char* password) 
{
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword(password);
    
    ArduinoOTA.onStart([this]() 
    { 
        this->onStart(); 
    });
    
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) 
    { 
        this->onProgress(progress, total); 
    });
    
    ArduinoOTA.onEnd([this]() 
    { 
        this->onEnd(); 
    });
    
    ArduinoOTA.onError([this](ota_error_t error) 
    { 
        this->onError(error); 
    });

    ArduinoOTA.begin();
    _initialized = true;
}

void OTAManager::handle() 
{
    ArduinoOTA.handle();
}

void OTAManager::onStart() 
{
    _display.displayText("OTA Update Start");
}

void OTAManager::onProgress(unsigned int progress, unsigned int total) 
{
    char progressStr[32];
    sprintf(progressStr, "Progress: %u%%", (progress / (total / 100)));
    _display.displayText(progressStr);
}

void OTAManager::onEnd() 
{
    _display.displayLines({"OTA Update Complete", "Gonna reset", "the device"});
    delay(5000);
    ESP.restart();
}

void OTAManager::onError(ota_error_t error) 
{
    char errorStr[32];
    sprintf(errorStr, "Error[%u]: ", error);
    _display.displayText(errorStr);
} 