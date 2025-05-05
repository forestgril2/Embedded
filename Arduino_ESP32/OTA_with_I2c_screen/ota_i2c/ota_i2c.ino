#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include "src/config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);

// Display helper functions
void displayStatus(const char* status) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.println(status);
  display.display();
}

// Blink handler
void handleBlink(AsyncWebServerRequest *request) {
  if (request->hasParam("count")) {
    String countStr = request->getParam("count")->value();
    int count = countStr.toInt();
    request->send(200, "text/plain", "Blinking " + String(count) + " times");

    for (int i = 0; i < count; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
      esp_task_wdt_reset();  // Reset watchdog timer
    }
  } else {
    request->send(400, "text/plain", "Missing 'count' parameter");
  }
}

// Status handler
void handleStatus(AsyncWebServerRequest *request) {
  String status = "{\"version\":\"" + String(VERSION) + "\",";
  status += "\"uptime\":" + String(millis() / 1000) + ",";
  status += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  status += "\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  request->send(200, "application/json", status);
}

// Root page with form
void handleRoot(AsyncWebServerRequest *request) {
  String html = "<html><body>";
  html += "<h1>ESP32 Control Panel</h1>";
  html += "<form action=\"/blink\">";
  html += "Blink count: <input name=\"count\" type=\"number\" min=\"1\" max=\"10\">";
  html += "<input type=\"submit\" value=\"Blink\">";
  html += "</form>";
  html += "<p><a href=\"/status\">System Status</a></p>";
  html += "</body></html>";
  request->send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize watchdog timer
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    displayStatus("Display Error!");
    delay(2000);
    ESP.restart();
  }
  display.clearDisplay();
  displayStatus("Starting...");
  display.display();

  WiFiManager wm;
  wm.setConfigPortalTimeout(WIFI_CONFIG_TIMEOUT);
  bool ok = wm.autoConnect(WIFI_AP_NAME);

  if (!ok) {
    Serial.println("Failed to connect. Restarting...");
    displayStatus("WiFi Failed!");
    delay(3000);
    ESP.restart();
  }

  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
  displayStatus("WiFi Connected!");

  // Async server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/blink", HTTP_GET, handleBlink);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/display", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text")) {
      String text = request->getParam("text")->value();
      displayStatus(text.c_str());
      request->send(200, "text/plain", "Text displayed: " + text);
    } else {
      request->send(400, "text/plain", "Missing 'text' parameter");
    }
  });

  server.begin();
  Serial.println("HTTP server started");

  // OTA Setup
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD); 
  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA update");
    displayStatus("OTA Update...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA update");
    displayStatus("OTA Complete!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char progressStr[32];
    snprintf(progressStr, sizeof(progressStr), "OTA: %u%%", (progress / (total / 100)));
    displayStatus(progressStr);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    char errorStr[32];
    snprintf(errorStr, sizeof(errorStr), "OTA Error: %u", error);
    displayStatus(errorStr);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  displayStatus("System Ready!");
}

void loop() {
  esp_task_wdt_reset();  // Reset watchdog timer
  ArduinoOTA.handle();
}