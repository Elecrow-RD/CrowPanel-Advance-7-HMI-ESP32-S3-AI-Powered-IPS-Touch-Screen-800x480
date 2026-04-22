#pragma once

#include <Arduino.h>  // Ensure the Arduino core library is included first

#ifdef __cplusplus     // Restrict to C++ environment
#include <vector>      // Safely include STL library
using namespace std;   // Add namespace declaration

#include "Audio.h"
#include <WiFi.h>
#include <TCA9534.h>
#include "ui.h"

extern Audio audio;

void audioInit();
void audioLoop();
void playNextSong();
void playPreviousSong();
void togglePlayPause();
bool isAudioPlaying();

#endif
