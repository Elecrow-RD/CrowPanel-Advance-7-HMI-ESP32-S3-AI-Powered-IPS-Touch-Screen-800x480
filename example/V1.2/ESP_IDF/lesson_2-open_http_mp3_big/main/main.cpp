#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <Wire.h>
#include "TCA9534.h"

const char *ssid = "YOUR-SSID";
const char *password = "YOUR-PASSWORD";

const char *audioUrl = "http://music.163.com/song/media/outer/url?id=2086327879.mp3";

#define BCLK 5
#define LRC 6
#define DOUT 4

Audio audio;
TCA9534 ioex;

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
  audio.setVolume(100);        // Set the volume level
  // Connect to the audio URL and start playback
  audio.connecttohost(audioUrl);
}

void init_big(){
  pinMode(19, OUTPUT);

  Wire.begin(15, 16);
  delay(50);

  ioex.attach(Wire);
  ioex.setDeviceAddress(0x18);
  ioex.config(1, TCA9534::Config::OUT);
  ioex.config(2, TCA9534::Config::OUT);
  ioex.config(3, TCA9534::Config::OUT);
  ioex.config(4, TCA9534::Config::OUT);


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