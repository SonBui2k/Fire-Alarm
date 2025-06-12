#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// Firebase libraries
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Network credentials
#define WIFI_SSID "Thai Son T2"
#define WIFI_PASSWORD "87654321"

// Firebase credentials
const char* API_KEY = "AIzaSyAnlG8Eex-84yvJweI4x379_zFanmaGANQ";
const char* DATABASE_URL = "https://fire-alarm-67-default-rtdb.asia-southeast1.firebasedatabase.app/";

// Sensor pins
#define MQ2_PIN 34        // Analog pin for MQ-2 (Gas/Smoke)
#define DHT_PIN 25         // Digital pin for DHT11
#define DHTTYPE DHT11     // DHT sensor type

// Thresholds
#define GAS_SMOKE_THRESHOLD 1000  // Ngưỡng khí ga/khói (điều chỉnh dựa trên thử nghiệm)
#define TEMP_THRESHOLD 50         // Ngưỡng nhiệt độ (°C)
#define HUMIDITY_THRESHOLD 80     // Ngưỡng độ ẩm (%)

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// DHT object
DHT dht(DHT_PIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;
const long sendDataInterval = 5000; // Gửi dữ liệu mỗi 5 giây
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Setup Firebase credentials
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous sign-up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Sign-Up OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase Sign-Up Error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataInterval || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Read MQ-2 sensor
    int gasSmokeValue = analogRead(MQ2_PIN);
    Serial.print("Gas/Smoke Value: ");
    Serial.println(gasSmokeValue);

    // Read DHT11 sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Check for fire alarm conditions
    bool alarm = false;
    if (gasSmokeValue > GAS_SMOKE_THRESHOLD || temperature > TEMP_THRESHOLD || humidity > HUMIDITY_THRESHOLD) {
      alarm = true;
      Serial.println("ALARM: Fire hazard detected!");
    } else {
      Serial.println("No fire hazard detected.");
    }

    // Upload data to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "sensors/gas_smoke", gasSmokeValue)) {
      Serial.println("Gas/Smoke data uploaded successfully!");
    } else {
      Serial.println("Failed to upload gas/smoke data!");
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "sensors/temperature", temperature)) {
      Serial.println("Temperature data uploaded successfully!");
    } else {
      Serial.println("Failed to upload temperature data!");
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "sensors/humidity", humidity)) {
      Serial.println("Humidity data uploaded successfully!");
    } else {
      Serial.println("Failed to upload humidity data!");
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setBool(&fbdo, "sensors/alarm", alarm)) {
      Serial.println("Alarm state uploaded successfully!");
    } else {
      Serial.println("Failed to upload alarm state!");
      Serial.println(fbdo.errorReason());
    }
  }
}