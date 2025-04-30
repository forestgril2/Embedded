#include <WiFiManager.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoOTA.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int ledPin = 12;  // GPIO12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);

// Function to display text on OLED
void displayText(const char* text, int line = 0) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, line * 10);
  display.setTextColor(SSD1306_WHITE);
  display.println(text);
  display.display();
}

// Function to display multiple lines
void displayLines(const char* line1, const char* line2 = "", const char* line3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.println(line1);
  display.setCursor(0, 10);
  display.println(line2);
  display.setCursor(0, 20);
  display.println(line3);
  display.display();
}

// Blink handler
void handleBlink(AsyncWebServerRequest *request) {
  if (request->hasParam("count")) {
    String countStr = request->getParam("count")->value();
    int count = countStr.toInt();
    request->send(200, "text/plain", "Blinking " + String(count) + " times");

    for (int i = 0; i < count; i++) {
      digitalWrite(ledPin, HIGH);
      delay(300);
      digitalWrite(ledPin, LOW);
      delay(300);
    }
  } else {
    request->send(400, "text/plain", "Missing 'count' parameter");
  }
}

// Root page with form
void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html",
                "<form action=\"/blink\">"
                "Blink count: <input name=\"count\" type=\"number\">"
                "<input type=\"submit\" value=\"Blink\">"
                "</form>");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.display();

  // Display initial commit info
  displayLines("Firmware Info:", 
               "Version: 1.0",
               "Ready to start");
  delay(3000);

  WiFiManager wm;
  bool ok = wm.autoConnect("ESP32-Setup");

  if (!ok) {
    displayText("WiFi Connect Failed");
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  displayText("WiFi Connected!");
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Async server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/blink", HTTP_GET, handleBlink);
  server.on("/display", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text")) {
      String text = request->getParam("text")->value();
      displayText(text.c_str());
      request->send(200, "text/plain", "Text displayed: " + text);
    } else {
      request->send(400, "text/plain", "Missing 'text' parameter");
    }
  });

  server.begin();
  Serial.println("HTTP server started");

  // OTA Setup
  ArduinoOTA.setHostname("esp32-blinker");
  ArduinoOTA.setPassword("haslo123"); 
  
  ArduinoOTA.onStart([]() {
    displayText("OTA Update Start");
    Serial.println("Start OTA update");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char progressStr[32];
    sprintf(progressStr, "Progress: %u%%", (progress / (total / 100)));
    displayText(progressStr);
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onEnd([]() {
    displayLines("OTA Update Complete", "Please reset", "the device");
    Serial.println("\nEnd OTA update");
    // Wait for 5 seconds before restarting
    delay(5000);
    ESP.restart();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    char errorStr[32];
    sprintf(errorStr, "Error[%u]: ", error);
    displayText(errorStr);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  displayText("OTA Ready");
}

void loop() {
  ArduinoOTA.handle();
} 