#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"

// === Pin Configuration ===
#define DHTPIN 15      // DHT22 data pin
#define GASPIN 34      // MQ2 analog output
#define LDRPIN 35      // LDR analog output
#define LEDPIN 2       // Alert LED pin

// === WiFi Settings for Wokwi ===
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// === ThingSpeak Settings ===
String apiKey = "ORUOWMBVCOSX70YQ";  // Replace with your own Write API Key
const char* server = "http://api.thingspeak.com/update";

// === DHT Sensor Object ===
DHTesp dht;

void setup() {
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);

  dht.setup(DHTPIN, DHTesp::DHT22);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password, 6);  // Use channel 6 for faster Wokwi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  // Read sensor values
  TempAndHumidity dhtData = dht.getTempAndHumidity();
  float temperature = dhtData.temperature;
  float humidity = dhtData.humidity;
  int lightLevel = analogRead(LDRPIN);
  int airQuality = analogRead(GASPIN);

  // Display readings
  Serial.println("esp3 - Sending Data...");
  Serial.printf("Temp: %.2f °C, Humidity: %.2f %%\n", temperature, humidity);
  Serial.printf("Light Level: %d, Air Quality: %d\n", lightLevel, airQuality);

  // LED logic with alert message
  if (temperature > 35 || airQuality > 600) {
    digitalWrite(LEDPIN, HIGH);  // Alert ON
    Serial.println("⚠️ ALERT: Threshold crossed!");
    if (temperature > 35) {
      Serial.printf("🔴 High Temperature: %.2f °C\n", temperature);
    }
    if (airQuality > 600) {
      Serial.printf("🔴 Poor Air Quality: %d\n", airQuality);
    }
  } else {
    digitalWrite(LEDPIN, LOW);   // Alert OFF
    Serial.println("✅ All readings are within normal range.");
  }

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(temperature, 2) +
                 "&field2=" + String(humidity, 2) +
                 "&field3=" + String(lightLevel) +
                 "&field4=" + String(airQuality);

    http.begin(url);
    int httpCode = http.GET();
    http.end();

    Serial.print("ThingSpeak Response: ");
    Serial.println(httpCode);
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(15000);  // Wait 15 seconds before next update
}
