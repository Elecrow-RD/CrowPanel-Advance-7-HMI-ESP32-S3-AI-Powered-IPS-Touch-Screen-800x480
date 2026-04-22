#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "wifi_connect.h"
#include "weather.h"

#include "ui.h"
#include "Weather_Icon/Sunny_Icon.h"
#include "Weather_Icon/Cloudy_Icon.h"
#include "Weather_Icon/Rainy_Icon.h"
#include "Weather_Icon/Snowy_Icon.h"
#include "Weather_Icon/Foggy_Icon.h"
#include <string.h>

static const String apiKey = "your-API-Key"; // As for how to apply, you can refer to the "ReadMe. md" document located in the same path as the code
static const String cityID = "1795563";// Shenzhen
// static const String cityID = "5128581";// New York
static String serverPath = "http://api.openweathermap.org/data/2.5/weather?id=" + cityID + "&appid=" + apiKey;

static unsigned long lastWeatherUpdate = 0;
const long weatherUpdateInterval = 600000; // 10 minutes

void getWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  HTTPClient http;
  http.begin(serverPath);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    float tempC = doc["main"]["temp"].as<float>() - 273.15;
    int humidity = doc["main"]["humidity"];
    float windSpeed = doc["wind"]["speed"];
    const char* condition = doc["weather"][0]["main"];
    const char* cityName = doc["name"]; // City name

    char tempStr[10]; snprintf(tempStr, sizeof(tempStr), "%d°C", int(tempC));
    char humStr[10];  snprintf(humStr, sizeof(humStr), "%d%%", humidity);
    char windStr[20]; snprintf(windStr, sizeof(windStr), "%.1fm/s", windSpeed);
    char locationStr[50]; snprintf(locationStr, sizeof(locationStr), "%s", cityName);

    lv_label_set_text(ui_WeatherLabel, condition);
    lv_label_set_text(ui_TempLabel, tempStr);
    lv_label_set_text(ui_HumiLabel, humStr);
    lv_label_set_text(ui_WindLabel, windStr);
    lv_label_set_text(ui_LocalLabel, locationStr); // Set location label

    Serial.printf("Update successful! %s: Weather: %s---Temperature: %.1f°C  Humidity: %.1f%%  Wind Speed: %.1fm/s\n", 
                 locationStr, condition, tempStr, humStr, windStr);

    updateWeatherIcon(condition);

    http.end();
    delay(5);
  } else {
    Serial.printf("HTTP request error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
  }
}

void handleWeatherUpdate(unsigned long currentMillis) {
  if (currentMillis - lastWeatherUpdate >= weatherUpdateInterval) {
    lastWeatherUpdate = currentMillis;
    getWeatherData();
  }
}

void updateWeatherIcon(const char* weatherCondition) {
  if (strcmp(weatherCondition, "Clear") == 0) {
    lv_img_set_src(ui_WeatherImage, &Sunny_Icon);
  } else if (strcmp(weatherCondition, "Clouds") == 0) {
    lv_img_set_src(ui_WeatherImage, &Cloudy_Icon);
  } else if (strcmp(weatherCondition, "Rain") == 0 || 
             strcmp(weatherCondition, "Drizzle") == 0) {
    lv_img_set_src(ui_WeatherImage, &Rainy_Icon);
  } else if (strcmp(weatherCondition, "Snow") == 0) {
    lv_img_set_src(ui_WeatherImage, &Snowy_Icon);
  } else if (!strcmp(weatherCondition, "Fog") || 
             !strcmp(weatherCondition, "Mist") || 
             !strcmp(weatherCondition, "Haze")) {
    lv_img_set_src(ui_WeatherImage, &Foggy_Icon);
  } else {
    lv_img_set_src(ui_WeatherImage, &Sunny_Icon);  // Default to Sunny icon
  }
}
