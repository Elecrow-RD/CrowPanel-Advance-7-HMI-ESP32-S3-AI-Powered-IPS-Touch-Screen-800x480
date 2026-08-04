#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>

#include "LovyanGFX_Driver.h"

#define CE_PIN  20
#define CSN_PIN 19

#define HSPI_MISO  4
#define HSPI_MOSI  6
#define HSPI_SCLK  5
#define HSPI_SS    19

LGFX gfx;
LGFX_Sprite textBuffer(&gfx);

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

SPIClass* hspi = nullptr;

/*
Function function: Display text on the screen
    lcd_w:  Product horizontal axis resolution
    lcd_h： Product vertical axis resolution
    x：     Screen displays the starting horizontal axis
    y：     Screen displays the starting vertical axis
    text：  The text content displayed on the screen
*/
/** @brief Render the latest transmitter status in a full-screen sprite. */
void show_test(int lcd_w, int lcd_h, int x, int y, const char * text)
{
  if (textBuffer.getBuffer() != nullptr) {
    textBuffer.fillSprite(TFT_BLACK);
    textBuffer.setTextSize(3);
    textBuffer.setTextColor(TFT_RED);
    textBuffer.setCursor(0, 5);
    textBuffer.print(text);
    textBuffer.pushSprite(280, 225);
    return;
  }
  gfx.startWrite();
  gfx.fillRect(160, 215, 600, 55, TFT_BLACK);
  gfx.setTextSize(3);
  gfx.setTextColor(TFT_RED);
  gfx.setCursor(x, y);
  gfx.print(text);
  gfx.endWrite();
}

// *********************************************************
/** @brief Probe an I2C address. @return true when acknowledged. */
bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Wrapper function for sending I2C commands
/** @brief Send one startup command to the board controller. */
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
// *********************************************************

const byte address[6] = "00001";
/** @brief Initialize board power, LCD, HSPI and the nRF24L01 transmitter. */
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
  // Start sending command 0 to address 0x30
  sendI2CCommand(0);  // 0 is the brightest backlight.    / 245 backlight off   (0-245)

  // Init Display
  gfx.init();
  gfx.initDMA();
  gfx.fillScreen(TFT_BLACK);
  textBuffer.setColorDepth(16);
  textBuffer.setPsram(false);
  textBuffer.createSprite(360, 36);
  show_test(800, 480, 250, 230, "RF24 STARTING...");

  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  hspi = new SPIClass(HSPI); // Initialize wireless module SPI
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  if (!radio.begin(hspi)) {
    Serial.println(F("radio hardware is not responding!!"));
    show_test(800, 480, 180, 230, "RF24 INIT FAILED");
    while (1) {}  // hold in infinite loop
  }
  else
  {
    Serial.println(F("radio hardware is OK!!"));
    show_test(800, 480, 250, 230, "RF24 READY");
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);  //RF24_250KBPS  RF24_1MBPS  RF24_2MBPS
  radio.setChannel(50);
  radio.stopListening();
}

int i = 0;
/** @brief Transmit the lesson payload and report the send result. */
void loop() {
  Serial.println(F("SEND !!"));
  String str = "SENDING...";
  str += String(i);
  show_test(800, 480, 300, 230, str.c_str());
  i++;
  const char text[] = "Hello World I:";
  radio.write(&text, sizeof(text));
  delay(1000);
}
