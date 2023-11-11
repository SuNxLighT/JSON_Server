#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "SuN";
const char* password = "1212312121";
const char* serverUrl = "http://192.168.140.200:6077/sensors";

WiFiClient client;
HTTPClient http;
DHT dht(D4, DHT11);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  connectToWiFi();
  dht.begin();
  timeClient.setTimeOffset(3600);
  timeClient.begin();
}

void loop() {
  static unsigned long lastTime = 0;
  unsigned long timerDelay = 15000;

  if ((millis() - lastTime) > timerDelay) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    DynamicJsonDocument jsonDocument(200);
    jsonDocument["humidity"] = humidity;
    jsonDocument["temperature"] = temperature;

    timeClient.update();
    jsonDocument["timestamp"] = timeClient.getFormattedTime();

    String jsonData;
    serializeJson(jsonDocument, jsonData);

    if (WiFi.status() == WL_CONNECTED) {
      sendPostRequest(jsonData);
    } else {
      connectToWiFi();
    }

    lastTime = millis();
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void sendPostRequest(String jsonData) {
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    String payload = http.getString();
    Serial.println("Returned payload:");
    Serial.println(payload);
  } else {
    Serial.println("Error on sending POST: " + String(httpResponseCode));
  }

  http.end();
}
