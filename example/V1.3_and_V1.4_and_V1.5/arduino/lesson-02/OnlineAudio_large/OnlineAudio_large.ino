#include <Wire.h>
#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"

#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6

Audio audio;
WiFiMulti wifiMulti;

String ssid     = "elecrow888";
String password = "elecrow2014";

bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Wrapper function for sending I2C commands
void sendI2CCommand(uint8_t command) {
  uint8_t error;
  // Start sending commands to the specified address
  Wire.beginTransmission(0x30);
  // Send command
  Wire.write(command);
  //  End transmission and return status
  error = Wire.endTransmission();

  if (error == 0) {
    Serial.print("command 0x");
    Serial.print(command, HEX);
    Serial.println(" Sent successfully");
  } else {
    Serial.print("Command sent error, error code:");
    Serial.println(error);
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(15, 16);
  delay(50);

  while (1) {
    if (i2cScanForAddress(0x30) && i2cScanForAddress(0x5D)) {
      Serial.print("The microcontroller is detected: address 0x");
      Serial.println(0x30, HEX);
      Serial.print("The microcontroller is detected: address 0x");
      Serial.println(0x5D, HEX);
      break;
    } else {
      Serial.print("No microcontroller was detected: address 0x");
      Serial.println(0x30, HEX);
      Serial.print("No microcontroller was detected: address 0x");
      Serial.println(0x5D, HEX);
      //Prevent the microcontroller did not start to adjust the bright screen
      sendI2CCommand(250);    // 250 : Activate touch screen
      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      
      delay(120);
      pinMode(1, INPUT);

      delay(100);
    }
  }
  // Start sending command 248 to address 0x30
  sendI2CCommand(248); // 248 : Turn on the speaker

  Serial.printf("[LINE--%d]\n", __LINE__);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    wifiMulti.run();
  }
  Serial.printf("[LINE--%d]\n", __LINE__);
  Serial.println("--- WIFI_CONNECTED ---");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(20);  // 0...21

  // Choose the URL of the music you want to play
  audio.connecttohost("http://music.163.com/song/media/outer/url?id=1391891631.mp3"); // Empire state of mine.mp3
  Serial.printf("[LINE--%d]\t ready to play!!\n", __LINE__);
}

void loop() {
  audio.loop();
  if (Serial.available()) {  // put streamURL in serial monitor
    audio.stopSong();
    String r = Serial.readString();
    r.trim();
    if (r.length() > 5) audio.connecttohost(r.c_str());
    log_i("free heap=%i", ESP.getFreeHeap());
  }
}
