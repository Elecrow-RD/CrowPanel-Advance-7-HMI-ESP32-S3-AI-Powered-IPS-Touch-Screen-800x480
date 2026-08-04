#include <Arduino.h>
#include <ESP_I2S.h>
#include <Wire.h>

/*---------------------------------------------------------------
 * Board audio-control bus
 * The audio controller is configured through I2C before recording
 * and playback so the microphone and amplifier paths are selected
 * at the right time.
 *--------------------------------------------------------------*/

// I2C pins used by the supplied CrowPanel Advance HMI audio example.
constexpr int I2C_SDA = 15;
constexpr int I2C_SCL = 16;
constexpr uint8_t AUDIO_CTRL_ADDR = 0x30;

/*---------------------------------------------------------------
 * Microphone and speaker pins
 * The on-board PDM microphone is read first, then the recording is
 * sent to the I2S amplifier and speaker.
 *--------------------------------------------------------------*/

// On-board PDM microphone pins: clock and data.
constexpr int MIC_CLK = 19;
constexpr int MIC_DATA = 20;

// On-board I2S amplifier pins: bit clock, word clock and audio data.
constexpr int SPK_BCLK = 5;
constexpr int SPK_LRCLK = 6;
constexpr int SPK_DATA = 4;

/*---------------------------------------------------------------
 * Audio format and buffer settings
 * The demo records a short 16-bit mono clip and converts it to a
 * stereo buffer because the board speaker is driven from one channel.
 *--------------------------------------------------------------*/

constexpr uint32_t SAMPLE_RATE = 16000;
constexpr uint32_t RECORD_SECONDS = 5;
constexpr float PLAYBACK_GAIN = 8.0f;
constexpr size_t WAV_HEADER_SIZE = 44;

// Shared I2S peripheral wrapper used for both microphone input and speaker output.
I2SClass audio;

/**
 * @brief Send one command byte to the board audio controller.
 *
 * The controller selects the active audio path and mutes or unmutes
 * the amplifier. This helper keeps the I2C transaction in one place.
 *
 * @param command Command byte accepted by the board controller.
 * @return true when the controller acknowledges the command.
 * @return false when the I2C transaction fails.
 *
 * Called before recording, before playback and after playback cleanup.
 */
bool sendAudioCommand(uint8_t command) {
  Wire.beginTransmission(AUDIO_CTRL_ADDR);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

/**
 * @brief Apply playback gain while preventing 16-bit overflow.
 *
 * Recorded microphone samples are quiet on this board. The gain makes
 * the short recording easier to hear, and the clipping limits prevent
 * integer overflow when a loud sample is amplified.
 *
 * @param sample Original signed 16-bit PCM sample.
 * @return Amplified signed 16-bit PCM sample.
 *
 * Called while building the stereo playback buffer.
 */
int16_t amplify(int16_t sample) {
  int32_t value = static_cast<int32_t>(sample * PLAYBACK_GAIN);
  if (value > INT16_MAX) value = INT16_MAX;
  if (value < INT16_MIN) value = INT16_MIN;
  return static_cast<int16_t>(value);
}

/**
 * @brief Record audio from the microphone and play it through the speaker.
 *
 * The function first enables the microphone path, records a five-second
 * WAV buffer, converts mono PCM samples to the speaker channel, then
 * unmutes the amplifier and writes the buffer to the I2S peripheral.
 *
 * @return true when all playback bytes are written.
 * @return false when initialization, recording or memory allocation fails.
 *
 * Called once during startup and again whenever the user sends r or R
 * from the Serial Monitor.
 */
bool recordAndPlay() {
  size_t wavSize = 0;

  // Command 2 selects/enables the microphone path in the supplied example.
  sendAudioCommand(2);
  sendAudioCommand(249);  // Keep the speaker muted while recording.

  audio.setPinsPdmRx(MIC_CLK, MIC_DATA);
  if (!audio.begin(I2S_MODE_PDM_RX, SAMPLE_RATE,
                   I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("ERROR: PDM microphone initialization failed.");
    return false;
  }

  Serial.println("Recording for 5 seconds...");
  uint8_t *wav = audio.recordWAV(RECORD_SECONDS, &wavSize);
  audio.end();

  if (wav == nullptr || wavSize <= WAV_HEADER_SIZE) {
    Serial.println("ERROR: Recording failed (check PSRAM/memory). ");
    free(wav);
    return false;
  }
  Serial.printf("Recording complete: %u bytes\n", static_cast<unsigned>(wavSize));

  // recordWAV returns mono 16-bit PCM after a standard 44-byte WAV header.
  uint8_t *monoBytes = wav + WAV_HEADER_SIZE;
  size_t monoSize = wavSize - WAV_HEADER_SIZE;
  size_t sampleCount = monoSize / sizeof(int16_t);
  size_t stereoSize = sampleCount * 2 * sizeof(int16_t);

  int16_t *stereo = static_cast<int16_t *>(
      heap_caps_malloc(stereoSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (stereo == nullptr) {
    // Fall back to internal RAM if PSRAM is not exposed as a separate heap.
    stereo = static_cast<int16_t *>(malloc(stereoSize));
  }
  if (stereo == nullptr) {
    Serial.println("ERROR: Playback buffer allocation failed.");
    free(wav);
    return false;
  }

  const int16_t *mono = reinterpret_cast<const int16_t *>(monoBytes);
  for (size_t i = 0; i < sampleCount; ++i) {
    stereo[i * 2] = 0;                 // Left channel is unused.
    stereo[i * 2 + 1] = amplify(mono[i]);  // Speaker is on the right channel.
  }

  audio.setPins(SPK_BCLK, SPK_LRCLK, SPK_DATA);
  if (!audio.begin(I2S_MODE_STD, SAMPLE_RATE,
                   I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("ERROR: I2S speaker initialization failed.");
    heap_caps_free(stereo);
    free(wav);
    return false;
  }

  Serial.println("Playing the recording...");
  sendAudioCommand(248);  // Unmute the on-board amplifier.
  size_t bytesWritten = audio.write(reinterpret_cast<uint8_t *>(stereo), stereoSize);
  delay(20);              // Let the final DMA samples leave the peripheral.
  sendAudioCommand(249);  // Mute again to avoid a pop/noise floor.
  audio.end();

  heap_caps_free(stereo);
  free(wav);
  sendAudioCommand(3);    // Restore the audio-control state used by the factory demo.

  Serial.printf("Playback complete: %u/%u bytes written.\n",
                static_cast<unsigned>(bytesWritten),
                static_cast<unsigned>(stereoSize));
  return bytesWritten == stereoSize;
}

/**
 * @brief Initialize serial output, I2C control and the first recording cycle.
 *
 * Called once after reset. The first recording starts automatically so
 * the lesson has an immediate, observable result after flashing.
 */
void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.println("\n5-second microphone record/playback demo");
  Serial.println("Send r in Serial Monitor to run it again.");
  recordAndPlay();
}

/**
 * @brief Wait for a serial command that repeats the record/playback test.
 *
 * The loop keeps the device idle after playback. Sending r or R from
 * the Serial Monitor clears any extra input bytes and starts a fresh
 * five-second recording cycle.
 */
void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'r' || command == 'R') {
      while (Serial.available()) Serial.read();
      recordAndPlay();
    }
  }
  delay(10);
}
