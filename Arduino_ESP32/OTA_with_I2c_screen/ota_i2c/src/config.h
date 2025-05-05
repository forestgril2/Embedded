#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>

// WiFi Configuration
#define WIFI_AP_NAME "ESP32-Setup"
#define WIFI_CONFIG_TIMEOUT 180  // 3 minutes timeout

// OTA Configuration
#define OTA_HOSTNAME "esp32-blinker"
#define OTA_PASSWORD "haslo123"  // Change this in production!

// Hardware Configuration
#define DEFAULT_LED_PIN 2  // Default LED pin
#define LED_PIN_EEPROM_ADDR 0  // EEPROM address to store LED pin
#define LED_ACTIVE_LOW true  // Set to true if LED is active low
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// System Configuration
#define WDT_TIMEOUT 30  // Watchdog timeout in seconds
#define VERSION "1.0.0"

// Helper functions for LED control
inline void setLedOn(int pin) {
    digitalWrite(pin, LED_ACTIVE_LOW ? LOW : HIGH);
}

inline void setLedOff(int pin) {
    digitalWrite(pin, LED_ACTIVE_LOW ? HIGH : LOW);
}

// Function to get LED pin from EEPROM or return default
inline int getLedPin() {
    static bool eepromInitialized = false;
    if (!eepromInitialized) {
        EEPROM.begin(1);  // Initialize EEPROM with 1 byte
        eepromInitialized = true;
    }
    
    int pin = EEPROM.read(LED_PIN_EEPROM_ADDR);
    // If pin is 0 or invalid, return default
    if (pin <= 0 || pin >= 40) {
        pin = DEFAULT_LED_PIN;
        // Save default to EEPROM
        EEPROM.write(LED_PIN_EEPROM_ADDR, pin);
        EEPROM.commit();
    }
    return pin;
}

// Function to save LED pin to EEPROM
inline void saveLedPin(int pin) {
    if (pin > 0 && pin < 40) {  // Validate pin number
        static bool eepromInitialized = false;
        if (!eepromInitialized) {
            EEPROM.begin(1);
            eepromInitialized = true;
        }
        EEPROM.write(LED_PIN_EEPROM_ADDR, pin);
        EEPROM.commit();
    }
}

#endif // CONFIG_H 