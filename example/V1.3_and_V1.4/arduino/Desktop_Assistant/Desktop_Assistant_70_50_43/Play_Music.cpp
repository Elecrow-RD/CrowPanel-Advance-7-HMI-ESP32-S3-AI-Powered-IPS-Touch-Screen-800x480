#include "Play_Music.h"
#include "wifi_connect.h"

// Hardware configuration
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6

// Audio control
Audio audio;

// Playlist management
// Add the songs you want to listen to here. We are using NetEase Cloud Music links – simply replace the "id=xxxx" part with the corresponding xxxx.
const char* playlist[] = {
    "http://music.163.com/song/media/outer/url?id=4164331.mp3",   // Bye Bye Bye
    "http://music.163.com/song/media/outer/url?id=1384450197.mp3",// Remember Our Summer
    "http://music.163.com/song/media/outer/url?id=1464325108.mp3",// Mood
    "http://music.163.com/song/media/outer/url?id=2071452224.mp3",// Take Me Hand
    "http://music.163.com/song/media/outer/url?id=1473214690.mp3",// One day
};

// Corresponding song name array
const char* songNames[] = {
    "Bye Bye Bye",
    "Remember Our Summer",
    "Mood",
    "Take Me Hand",
    "One day"
};

uint8_t currentTrack = 0;
bool isPlaying = false;     // Whether music is currently playing
bool hasConnected = false;  // Whether connecttohost() has been called

void audioInit() {
    // Initialize hardware
    Wire.begin(15, 16);
    delay(50);

    Wire.beginTransmission(0x30);            // 7-digit address 0x30
    Wire.write(248);                        // Send command 248
    uint8_t error = Wire.endTransmission();  // End transmission and return status

    if (error == 0) {
    Serial.println("Command 248 successfully sent");
    } else {
    Serial.print("Command sending error, error code：");
    Serial.println(error);
    }

    // Connect to WiFi
    // connectToWiFi();  

    // Initialize audio
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(20); // Volume level: 0...21

    lv_label_set_text(ui_PlayOrPauseLabel, "Play");       // Initial button label: "Play"
    lv_label_set_text(ui_MusicStatusLabel, "PAUSING");    // Initial status: "PAUSING"
    lv_label_set_text(ui_SongNameLabel, songNames[currentTrack]);  // Show first song name
}

void audioLoop() {
    audio.loop();
}

void updateUI() {
    if (isPlaying) {
        lv_label_set_text(ui_PlayOrPauseLabel, "Pause");
        lv_label_set_text(ui_MusicStatusLabel, "PLAYING");
    } else {
        lv_label_set_text(ui_PlayOrPauseLabel, "Play");
        lv_label_set_text(ui_MusicStatusLabel, "PAUSING");
    }
    // Update current song name
    lv_label_set_text(ui_SongNameLabel, songNames[currentTrack]);

    // Refresh
    lv_obj_invalidate(ui_PlayOrPauseLabel);
    lv_obj_invalidate(ui_MusicStatusLabel);
    lv_obj_invalidate(ui_SongNameLabel);
}

void playCurrentTrack() {
    // Stop old stream (if any)
    audio.stopSong();
    // Connect and play current track
    audio.connecttohost(playlist[currentTrack]);
    hasConnected = true;
    isPlaying = true;
    updateUI();
}

void playNextSong() {
    currentTrack = (currentTrack + 1) % (sizeof(playlist)/sizeof(playlist[0]));
    hasConnected = false;    // Ensure playCurrentTrack reconnects
    playCurrentTrack();      // Switch song and auto play
    Serial.println("---------- Next Song ----------");
}

void playPreviousSong() {
    currentTrack = (currentTrack == 0)
        ? (sizeof(playlist)/sizeof(playlist[0]) - 1)
        : (currentTrack - 1);
    hasConnected = false;
    playCurrentTrack();
    Serial.println("---------- Previous Song ----------");
}

void togglePlayPause() {
    if (!hasConnected) {
        // Not connected yet, start playing directly
        playCurrentTrack();
        return;
    }
    // Already connected, just pause/resume
    audio.pauseResume();
    delay(50);
    isPlaying = audio.isRunning();
    updateUI();
    Serial.println(isPlaying ? "---------- PLAYING ----------"
                             : "---------- PAUSING ----------");
}

bool isAudioPlaying() {
    return isPlaying;
}
