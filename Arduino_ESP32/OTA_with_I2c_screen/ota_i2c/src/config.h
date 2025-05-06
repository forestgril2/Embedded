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
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// System Configuration
#define WDT_TIMEOUT 30  // Watchdog timeout in seconds
#define VERSION "1.0.0"

#endif // CONFIG_H 