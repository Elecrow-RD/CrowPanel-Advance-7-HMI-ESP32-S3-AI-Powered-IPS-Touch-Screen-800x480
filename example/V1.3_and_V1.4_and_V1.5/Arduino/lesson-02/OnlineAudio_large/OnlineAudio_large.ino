#include <Wire.h>
#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"

#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6

Audio audio;
WiFiMulti wifiMulti;

// Stores the credentials used by the Wi-Fi connection task.
String ssid     = "yanfa1";
String password = "1223334444yanfa";

/**
 * @brief Print status messages emitted by the audio library.
 * @param message Library message containing a category and detail string.
 * Called by the audio decoder whenever playback state changes.
 */
void audioInfo(Audio::msg_t message) {
  Serial.printf("[AUDIO] %s: %s\n", message.s, message.msg);
}

/**
 * @brief Check whether an I2C device acknowledges an address.
 * @param address Seven-bit I2C address to probe.
 * @return true when the device acknowledges; otherwise false.
 * Called during setup before sending board-control commands.
 */
bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

/**
 * @brief Send one board-control byte over I2C.
 * @param command Command byte accepted by the board controller.
 * Called during startup after both required I2C addresses are detected.
 */
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

/**
 * @brief Initialize I2C, Wi-Fi and I2S audio playback.
 *
 * The function stops in an explicit retry loop when required hardware,
 * network access or the stream connection is unavailable.
 */
void setup() {
  Serial.begin(115200);
  Audio::audio_info_callback = audioInfo;

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
  Serial.print("Connecting to WiFi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.printf("WiFi connected, IP: %s, RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());

  if (!audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
    Serial.println("ERROR: Failed to configure I2S pins");
    while (true) {
      delay(1000);
    }
  }
  audio.setVolume(20);  // 0...21

  // Choose the URL of the music you want to play
  if (!audio.connecttohost("http://music.163.com/song/media/outer/url?id=1391891631.mp3")) {
    Serial.println("ERROR: Failed to connect to the audio URL");
    while (true) {
      delay(1000);
    }
  }
  Serial.printf("[LINE--%d]\t ready to play!!\n", __LINE__);
}

/**
 * @brief Keep the decoder running and accept optional stream URLs.
 *
 * Audio.loop() must be called repeatedly; a URL typed into Serial replaces
 * the current stream and makes network playback observable without editing
 * the firmware.
 */
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
