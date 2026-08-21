#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <Wire.h>

// Wi-Fi credentials used by the demo to reach the HTTP MP3 stream.
// Change these values before sharing the firmware with end users.
const char *ssid      = "yanfa1";
const char *password  = "1223334444yanfa";

// HTTP MP3 stream URL. The ESP32 downloads this stream and sends decoded audio to I2S.
const char *audioUrl = "http://music.163.com/song/media/outer/url?id=1391891631.mp3";

// I2S audio output pins connected to the external audio amplifier/codec.
#define BCLK  5
#define LRC   6
#define DOUT  4

// Audio decoder/player object from the ESP32-audioI2S library.
Audio audio;

// Probe one I2C address and return true when a device acknowledges it.
bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Send one command byte to the onboard controller at I2C address 0x30.
// This controller is used for board-level functions such as touch and speaker power.
void sendI2CCommand(uint8_t command) {
  uint8_t error;

  // Start transmission to the onboard controller.
  Wire.beginTransmission(0x30);

  // Queue the command byte.
  Wire.write(command);

  // Finish transmission and collect the bus status.
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

// Connect to the configured Wi-Fi network before opening the HTTP audio stream.
void connent_wifi()
{
  Serial.printf("connect to %s ", ssid);
  WiFi.begin(ssid, password);

  // Wait here until the station is connected. The dots make progress visible on serial output.
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
}

// Configure I2S and start playback from the remote MP3 URL.
void play_http_mp3()
{
  audio.setPinout(BCLK, LRC, DOUT); // Route BCLK/LRCK/DOUT to the selected GPIO pins.
  audio.setVolume(20);              // Set the playback volume. Valid range depends on the library.

  // Connect to the audio URL and start decoding/playing the stream.
  audio.connecttohost(audioUrl);
  Serial.println("Playing audio from URL: " + String(audioUrl));
}

// Initialize board-specific peripherals required by the large CrowPanel board.
void init_big(){
  // GPIO19 is reserved as a simple output by this lesson template.
  pinMode(19, OUTPUT);

  // Initialize the I2C bus used by the onboard controller and touch controller.
  // SDA = GPIO15, SCL = GPIO16.
  Wire.begin(15, 16);
  delay(50);

  // Wait until both expected I2C devices respond:
  // 0x30: onboard controller, 0x5D: touch controller.
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

      // Wake/enable the touch controller and pulse GPIO1 to help the onboard controller recover.
      sendI2CCommand(250);    // 250: activate touch screen.
      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      
      delay(120);
      pinMode(1, INPUT);

      delay(100);
    }
  }

  // Enable the speaker path through the onboard controller.
  sendI2CCommand(248); // 248: turn on the speaker.
}

void setup()
{
  // Initialize serial output for status/debug messages.
  Serial.begin(115200);

  // Network must be ready before the HTTP stream can be opened.
  connent_wifi();

  // Bring up board-level I2C devices and speaker power.
  init_big();

  // Start MP3 playback.
  play_http_mp3();
}

void loop()
{
  // Keep the audio decoder fed. This must run frequently to avoid playback dropouts.
  audio.loop();
  delay(10);
}

// ESP-IDF entry point. It initializes Arduino compatibility and then runs the Arduino-style setup/loop.
extern "C" void app_main(void)
{
    initArduino();

    setup();

    while (true)
    {
        loop();
        delay(1);
    }
}
