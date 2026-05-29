// wifi_connect.cpp
#include "wifi_connect.h"
#include <Arduino.h>

const char* ssid     = "elecrow888"; // your WiFi Name
const char* password = "elecrow2014";// your WiFi password

void connectToWiFi() {
    Serial.print("Connecting to WiFi :");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nWiFi connected. IP address: ---");
    Serial.println(WiFi.localIP());
}




