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

  WiFiManager wm;
  bool ok = wm.autoConnect("ESP32-Setup");

  if (!ok) {
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Async server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/blink", HTTP_GET, handleBlink);
  server.on("/display", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text")) {
      String text = request->getParam("text")->value();
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.setTextColor(SSD1306_WHITE);
      display.println(text);
      display.display();
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
    Serial.println("Start OTA update");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA update");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void loop() {
  ArduinoOTA.handle();
}