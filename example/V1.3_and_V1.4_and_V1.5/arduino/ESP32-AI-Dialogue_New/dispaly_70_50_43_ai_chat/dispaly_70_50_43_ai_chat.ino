#include <WiFi.h>              // WiFi library for connecting and managing WiFi networks
#include <WiFiManager.h>       // Used to simplify the WiFi configuration process
#include <WebSocketsClient.h>  // It is used to implement WebSocket client functions
#include <base64.h>            // Used for Base64 encoding and decoding
#include <ArduinoJson.h>       // Used to process JSON data
#include <Wire.h>
#include "ESP_I2S.h"
#include <Button.h>
#include "ChatDialog.h" 
#include "Arduino.h"


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


ChatDialog chatDialog;
#define BUTTON_PIN 0        
Button button(BUTTON_PIN);

LGFX tft;
const char* websocket_server = "192.168.50.157";  // Your server IP address
const int websocket_port = 8765;
const char* websocket_path = "/";
String macAddress;
WebSocketsClient webSocket;

#define SAMPLE_RATE 16000  
#define SAMPLE_SIZE 1024   

bool isOpenMic = false;                        
bool isOpenSpk = false;                        
unsigned long isOpenMic_true_time = millis();  

QueueHandle_t queue;  

//--------------------------------------------------------------
// Task: Process the messages in the queue
void Task1(void *pvParameters) {
    String *receivedDataPtr;
    while (1) {
        if (xQueueReceive(queue, &receivedDataPtr, 0) == pdPASS) {
            chatDialog.addMessage(*receivedDataPtr, true);
            delete receivedDataPtr; 
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  
    }
}
//--------------------------------------------------------------
CustomI2S i2s;

void setup() {
  Serial.begin(115200);  
  Wire.begin(15, 16);

  init_screen();         

  queue = xQueueCreate(1000, sizeof(String));
  if (queue == NULL) {
      Serial.println("Queue creation failed");
      while (1); 
  }

  xTaskCreatePinnedToCore(
      Task1,    
      "Task1",  
      10000,    
      NULL,     
      1,        
      NULL,     
      0         
  );

  wifiServer();          
  connect_ws();          
  button.begin();        
}

void loop() {
  chatDialog.update();
  buttonWakeUpTask();
  change_status();
  webSocket.loop();     
  if (isOpenMic) {
    record_send_audio();  
  }
}

//--------------------------------------------------------------
// I2C Utility function
bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

void sendI2CCommand(uint8_t command) {
  Wire.beginTransmission(0x30);
  Wire.write(command);
  uint8_t error = Wire.endTransmission();
  if (error == 0) {
    Serial.printf("command 0x%02X Sent successfully\n", command);
  } else {
    Serial.printf("Command sent error, code:%d\n", error);
  }
}

//--------------------------------------------------------------
// Screen initialization
void init_screen() {
  Wire.begin(15, 16);
  delay(50);
  chatDialog.begin();
  while (1) {
    if (i2cScanForAddress(0x30) && i2cScanForAddress(0x5D)) {
      Serial.println("I2C devices detected.");
      break;
    } else {
      Serial.println("I2C devices not detected, retry...");
      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      delay(120);
      pinMode(1, INPUT);
      delay(100);
    }
  }
  sendI2CCommand(0);
  tft.startWrite();
}

//--------------------------------------------------------------
// Button Wake-Up Logic
void buttonWakeUpTask() {
  static unsigned long interruptStartTime = 0;  
  static bool waitingToOpenMic = false;         

  button.read();  

  if (button.pressed()) {
    if (isOpenSpk && !isOpenMic) {
      Serial.print("Interrupt!");
      isOpenSpk = false;
      i2s.end();             // 🔹 turn off speaker
      interruptStartTime = millis();  
      waitingToOpenMic = true;        
      String jsonString_i = createJsonString("interrupt_audio", "");
      webSocket.sendTXT(jsonString_i);

    } else if (!isOpenSpk && !isOpenMic) {
      Serial.print("Wake up!");
      String jsonString_i = createJsonString("wake_up", "");
      webSocket.sendTXT(jsonString_i);
      isOpenSpk = true;
      init_SPK();                // 🔹 Initialize the speaker
    }
  }

  if (waitingToOpenMic && (millis() - interruptStartTime >= 1000)) {
    isOpenMic = true;
    isOpenMic_true_time = millis();
    waitingToOpenMic = false;  
    init_MIC();                 // 🔹 Initialize the microphone
  }
}

//--------------------------------------------------------------
// WiFi
void wifiServer() {
  WiFiManager manager;
  macAddress = WiFi.macAddress();
  macAddress.replace(":", "");
  Serial.print("MAC Address: ");
  Serial.println(macAddress);

  String APName = "AI-" + macAddress;
  manager.autoConnect(APName.c_str());

  // WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
}

//--------------------------------------------------------------
// WebSocket
void connect_ws() {
  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket connected");
      {
        String jsonString = createJsonString("open_word", "");
        webSocket.sendTXT(jsonString);
        isOpenMic = true;
        isOpenMic_true_time = millis();
        init_MIC();            // 🔹 Turn on the MIC initially
      }
      break;
    case WStype_TEXT: {
      String rcv_word = (char*)payload;
      if (rcv_word.startsWith("USER:")) {
          chatDialog.addMessage(rcv_word.substring(5), false);
      } else if (rcv_word.startsWith("AI:")) {
          String *message = new String(rcv_word.substring(3));
          xQueueSend(queue, &message, portMAX_DELAY);
      } else if (rcv_word == "close_mic") {
          isOpenMic = false;
          i2s.end();       // 🔹 close MIC
          isOpenSpk = true;
          init_SPK();          // 🔹 Turn on the speaker
      } else if (rcv_word == "finish_tts") {
          delay(1000);
          String jsonString = createJsonString("re_process_audio", "");
          webSocket.sendTXT(jsonString);
          isOpenSpk = false;
          i2s.end();       // 🔹 turn off speaker

          init_MIC();          // 🔹 Reopen the MIC
          isOpenMic = true;
          isOpenMic_true_time = millis();
      }
      break;
    }
    case WStype_BIN: {
      Serial.printf("Received PCM audio data: %d bytes\n", length);

      if (!isOpenSpk) {
        Serial.println("Speaker not initialized. Discarding audio.");
        break;
      }

      if (length % 2 != 0) length--; // Ensure 16-bit alignment

      size_t samples = length / 2;
      size_t stereo_bytes = samples * 4;

      static int16_t* stereo_buffer = nullptr;
      static size_t stereo_buf_size = 0;

      // If the buffer is not large enough, reallocate it once (instead of using malloc/free every time)
      if (stereo_buf_size < stereo_bytes) {
        if (stereo_buffer) heap_caps_free(stereo_buffer);
        stereo_buffer = (int16_t*)heap_caps_malloc(stereo_bytes, MALLOC_CAP_SPIRAM);
        stereo_buf_size = stereo_bytes;
      }

      if (!stereo_buffer) {
        Serial.println("ERROR: Failed to allocate stereo buffer");
        break;
      }

      float amplification = 8.0;   // magnification times
      int16_t* input_samples = (int16_t*)payload;

      for (size_t i = 0; i < samples; i++) {
        int16_t sample = input_samples[i];

        // amplification
        float amplified = sample * amplification;
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        int16_t new_sample = (int16_t)amplified;

        // Convert to stereo sound
        stereo_buffer[i * 2]     = new_sample; // left channel
        stereo_buffer[i * 2 + 1] = new_sample; // right channel
      }

      size_t bytes_written = i2s.write((uint8_t*)stereo_buffer, stereo_bytes);
      if (bytes_written != stereo_bytes) {
        Serial.printf("Warning: Only wrote %d/%d bytes\n", bytes_written, stereo_bytes);
      } else {
        Serial.printf("Played %d samples (%d bytes)\n", samples, stereo_bytes);
      }

      break;
    }

  }
}


int warmupFrames = 0;   // Used for counting the first few frames
//--------------------------------------------------------------
// Initialize MIC
void init_MIC() {
  i2s.setPinsPdmRx(19, 20);
  if (!i2s.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("Failed to init MIC!");
    return;
  }
  Serial.println("Mic initialized.");
  warmupFrames = 0;
}

// Initialize SPK
void init_SPK() {
  i2s.setPins(5, 6, 4, -1);
  if (!i2s.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("Failed to init SPK!");
    return;
  }
  Serial.println("Speaker initialized.");
}


//--------------------------------------------------------------
// Record and send (using only ZCR for judgment)
// - Voice detection: If the ZCR value is greater than 0.01 for 7 consecutive times, it is determined as the presence of human voice
//--------------------------------------------------------------
void record_send_audio() {
  static unsigned long lastTime = 0;
  static unsigned long lastVoiceTime = 0;
  static bool isVoiceActive = false;
  static int zcrCount = 0;   // Count of consecutive satisfaction occurrences

  const int IGNORE_FRAMES = 30;   
  unsigned long currentTime = millis();

  if (isOpenMic && millis() - isOpenMic_true_time > 10000 && !isVoiceActive) {
    Serial.println("No voice 10s, sleep.");
    isOpenMic = false;
    i2s.end();
    String jsonString = createJsonString("timeout_no_stream", "");
    webSocket.sendTXT(jsonString);
    return;
  }

  if (currentTime - lastTime >= (1000.0 * SAMPLE_SIZE / SAMPLE_RATE)) {
    lastTime = currentTime;
    int16_t buffer[SAMPLE_SIZE] = {0};
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
      buffer[i] = (int16_t)i2s.read();
    }

    // ----------Calculate ZCR----------
    int zeroCrossings = 0;
    int16_t prevSample = buffer[0];
    for (int i = 1; i < SAMPLE_SIZE; i++) {
      int16_t sample = buffer[i];
      if ((sample > 0 && prevSample < 0) || (sample < 0 && prevSample > 0)) {
        zeroCrossings++;
      }
      prevSample = sample;
    }
    float zcr = (float)zeroCrossings / SAMPLE_SIZE;

    Serial.printf("ZCR=%.3f  zcrCount=%d  VoiceActive=%d\n", zcr, zcrCount, isVoiceActive);

    if (warmupFrames < IGNORE_FRAMES) {
      warmupFrames++;
    } else {
      // ---------- Voice detection logic ----------
      if (zcr > 0.01) {
        zcrCount++;
        if (zcrCount >= 3 && !isVoiceActive) {
          isVoiceActive = true;
          lastVoiceTime = currentTime;
          Serial.println(">>> Voice detected");
        }
      } else {
        zcrCount = 0; // Once it is not met, reset to zero.
      }

      // if (isVoiceActive) {
      //   lastVoiceTime = currentTime;
      // }

      if (isVoiceActive && currentTime - lastVoiceTime > 800) {
        Serial.println(">>> Voice ended");
        String jsonString = createJsonString("record_stream", "0x04");
        webSocket.sendTXT(jsonString);
        isVoiceActive = false;
        isOpenMic = false;
        i2s.end();
        isOpenSpk = true;
        init_SPK();
      }
    }

    // ---------- Always send audio ----------
    uint8_t* bytePtr = (uint8_t*)buffer;
    String base64Data = base64::encode(bytePtr, SAMPLE_SIZE * sizeof(int16_t));
    String jsonString = createJsonString("record_stream", base64Data);
    webSocket.sendTXT(jsonString);
  }
}



//--------------------------------------------------------------
// JSON tools
String createJsonString(const String& type, const String& data) {
  StaticJsonDocument<300> jsonDoc;
  jsonDoc["event"] = type;
  jsonDoc["mac_address"] = macAddress;
  jsonDoc["data"] = data;
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  return jsonString;  
}

// status icon
void change_status() {
  if (!isOpenMic && !isOpenSpk) {
      chatDialog.showMicOrSpk(3);
  } else if (isOpenMic && !isOpenSpk) {
      chatDialog.showMicOrSpk(1);
  } else if (!isOpenMic && isOpenSpk) {
      chatDialog.showMicOrSpk(2);
  }
}
