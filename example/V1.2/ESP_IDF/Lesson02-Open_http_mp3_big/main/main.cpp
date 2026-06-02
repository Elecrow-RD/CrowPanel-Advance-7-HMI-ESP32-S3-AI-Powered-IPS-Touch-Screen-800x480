#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <Wire.h>

const char *ssid = "elecrow888";
const char *password = "elecrow2014";

const char *audioUrl = "http://music.163.com/song/media/outer/url?id=1391891631.mp3";

#define BCLK 5
#define LRC  6
#define DOUT 4

Audio audio;

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

void connent_wifi()
{
  // Connect to WiFi
  Serial.printf("connect to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void play_http_mp3()
{
  audio.setPinout(BCLK,LRC,DOUT); // Adjust pins according to your setup
  audio.setVolume(20);        // Set the volume level
  // Connect to the audio URL and start playback
  audio.connecttohost(audioUrl);
}

void init_big(){
  pinMode(19, OUTPUT);

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

      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      
      delay(120);
      pinMode(1, INPUT);

      delay(100);
    }
  }
  // Start sending command 0x17 to address 0x30
  sendI2CCommand(0x17); // 0x17 : Turn on the speaker
}


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Connect to WiFi
  connent_wifi();

  init_big();

  play_http_mp3();
}


void loop()
{
  audio.loop();
  delay(10);
}


extern "C" void app_main()
{
  initArduino();
  setup();
  for (;;)
  {
    loop();
  }
}