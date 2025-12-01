#include <Wire.h>
#include <ESP_I2S.h>

// Microphone Pin Definitions
#define I2S_WS_MIC 2    // WS
#define I2S_SCK_MIC 19  // SCK
#define I2S_SD_MIC 20   // SD (Data entry)

// Speaker Pin Definitions
#define I2S_DOUT_SPK 4  // Speaker Data Output
#define I2S_LRC_SPK 6   // Audio Channel Clock（WS/LRC）
#define I2S_SPK_BCLK 5  // Time clock (BCLK)

// Sampling Configuration Parameters
const uint32_t SAMPLE_RATE = 16000;
const i2s_data_bit_width_t BITS = I2S_DATA_BIT_WIDTH_16BIT;
const int CHANNEL_STEREO = 2;
const int RECORD_SECONDS = 5;
const size_t REC_BUFFER_SIZE = SAMPLE_RATE * RECORD_SECONDS * (BITS / 8) * CHANNEL_STEREO;

class CustomI2S : public I2SClass{
  public:
    size_t write(uint8_t *buffer, size_t size) {
      size_t written = 0;
      size_t bytes_sent = 0;
      size_t bytes_to_send = 8000;
      last_error = ESP_FAIL;
      tx_chan = txChan();
      if (tx_chan == NULL) {
        return written;
      }
      while (written < size) {
        bytes_sent = 0;
        // if(written >= 26000)
        // {
          Wire.beginTransmission(0x30);
          Wire.write(248);
          Wire.endTransmission();
        // }
        esp_err_t err = i2s_channel_write(tx_chan, (char *)(buffer + written), bytes_to_send, &bytes_sent, _timeout);
        setWriteError(err);
        if (err != ESP_OK) {
          return written;
        }
        written += bytes_sent;
      }
      Wire.beginTransmission(0x30);
      // Wire.write(249);
      Wire.endTransmission();
      return written;
    }
  private:
    esp_err_t last_error;
    i2s_chan_handle_t tx_chan;
};

void Mic_init() {
  CustomI2S i2s;
  uint8_t *wav_buffer;
  size_t wav_size;

  // Initialize I2C and send a unmute command.
  Wire.beginTransmission(0x30);
  // Wire.write(249);
  Wire.endTransmission();

  Serial.println("Initializing the I2S bus...");

  // Configure the pins for audio input
  i2s.setPinsPdmRx(19, 20);

  // Initiate I2S with a 16 kHz frequency and 16-bit depth in mono mode.
  if (!i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("I2S initialization failed!");
    while (1);
  }

  Serial.println("Recording 5 seconds of audio data...");

  // Record 5 seconds of audio data
  wav_buffer = i2s.recordWAV(5, &wav_size);

  // Reconfigure I2S for output mode
  i2s.end();

  // Configure I2S outputs 5, 6, and 4 using the defined speaker pins.      
  i2s.setPins(I2S_SPK_BCLK, I2S_LRC_SPK, I2S_DOUT_SPK);

  // Start I2S with the same parameters, but switch to output mode and use stereo.
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("Initialization of I2S output mode failed!");
    while (1);
  }

  Serial.println("Playing recorded audio...");

  // Skip the WAV file header (typically 44 bytes)
  uint8_t *audio_data = wav_buffer + 44;
  size_t audio_size = wav_size - 44;

  // Create a stereo buffer (left channel muted, right channel playing)
  uint8_t *stereo_buffer = (uint8_t*)heap_caps_malloc(audio_size * 2, MALLOC_CAP_SPIRAM);
  if (stereo_buffer == NULL) {
    Serial.println("Memory allocation failed!");
    return;
  }

  // Define the amplification factor
  float amplification = 10.0;

  // Convert monaural data to stereo data (left channel as 0, right channel as amplified original data)
  for (size_t i = 0, j = 0; i < audio_size; i += 2, j += 4) {
    // Left channel audio data (muted)
    stereo_buffer[j] = 0;
    stereo_buffer[j + 1] = 0;

    // Acquire raw 16-bit audio samples
    int16_t sample = (int16_t)((audio_data[i+1] << 8) | audio_data[i]);

    // Magnify sample values and prevent overflow
    float amplified_sample = sample * amplification;
    if (amplified_sample > 32767) amplified_sample = 32767;
    if (amplified_sample < -32768) amplified_sample = -32768;

    // Convert back to bytes and store to the right channel
    int16_t new_sample = (int16_t)amplified_sample;
    stereo_buffer[j + 2] = new_sample & 0xFF;
    stereo_buffer[j + 3] = (new_sample >> 8) & 0xFF;
  }

  // Play audio data
  i2s.write(stereo_buffer, audio_size * 2);
  
  // Release memory
  heap_caps_free(stereo_buffer);
  free(wav_buffer);

  // End I2S
  i2s.end();

  Serial.println("Audio playback complete.");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
   
  Mic_init();
}

void loop() {
  // Empty loop, only executes the test once
}



