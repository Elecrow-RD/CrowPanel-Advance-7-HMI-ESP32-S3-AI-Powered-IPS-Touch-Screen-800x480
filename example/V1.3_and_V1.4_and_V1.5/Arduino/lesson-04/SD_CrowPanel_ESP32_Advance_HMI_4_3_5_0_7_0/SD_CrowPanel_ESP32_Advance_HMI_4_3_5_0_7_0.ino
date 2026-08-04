
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include "LovyanGFX_Driver.h"
#include "pins_config.h"

#define SD_MOSI 6
#define SD_MISO 4
#define SD_SCK  5
#define SD_CS   0 //The chip selector pin is not connected to IO
#define SD_SPI_FREQ 40000000

// The image name you stored on the SD card
#define IMAGE_1 "/1.bmp"
#define IMAGE_2 "/2.bmp"
#define IMAGE_3 "/3.bmp"
#define IMAGE_4 "/4.bmp"
#define IMAGE_5 "/5.bmp"

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass SD_SPI = SPIClass(FSPI);
#else
SPIClass SD_SPI = SPIClass(HSPI);
#endif
LGFX gfx;
LGFX_Sprite frame(&gfx);
static bool frame_ready = false;
static uint8_t bmp_line[3 * LCD_H_RES];

/** @brief Attach a 16-bit sprite to the display back buffer. @return true when the buffer is available. */
bool initFrameBuffer()
{
  frame.setColorDepth(16);
  frame.setBuffer(gfx._bus_instance.getBackBuffer(), LCD_H_RES, LCD_V_RES, 16);
  frame.setRotation(2);
  frame_ready = (frame.getBuffer() != nullptr);
  return frame_ready;
}

// Display text on the screen
/** @brief Draw a status message into the frame buffer and present it on the LCD. */
void show_test(int lcd_w, int lcd_h, int x, int y, const char * text) {
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextSize(3);
  gfx.setTextColor(TFT_RED);
  gfx.setCursor(x, y);
  gfx.print(text); 
}

/** @brief Probe an I2C address. @return true when acknowledged. */
bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Wrapper function for sending I2C commands
/** @brief Send one board power-control command over I2C. */
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


/** @brief Initialize board power, LCD, SD interface and the shared frame buffer. */
void setup()
{
  Serial.begin(115200); // set baud rate

  // Init Display
  gfx.setColorDepth(16);
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);
  gfx.endWrite();
  initFrameBuffer();
  delay(500);

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

  if (SD_init() == 0)
  {
    Serial.println("TF_Card initialization succeeded");
    show_test(LCD_H_RES, LCD_V_RES, 350, 230, "SD_Card OK");
    delay(3000);
  } else {
    Serial.println("TF card initialization failed");
    show_test(LCD_H_RES, LCD_V_RES, 350, 230, "SD_Card failed");
    delay(3000);
  }
  gfx.setRotation(2);
  gfx.fillScreen(TFT_BLACK);
  Serial.println( "----- Setup done -----" );
}

/** @brief Display the five configured BMP files in a repeating sequence. */
void loop()
{
 // Cycle printing each image
  Serial.println("Refreshing image...1");
  displayImage(SD, IMAGE_1, 800, 480);
  delay(5000);

  Serial.println("Refreshing image...2");
  displayImage(SD, IMAGE_2, 800, 480);
  delay(5000);

  Serial.println("Refreshing image...3");
  displayImage(SD, IMAGE_3, 800, 480);
  delay(5000);

  Serial.println("Refreshing image...4");
  displayImage(SD, IMAGE_4, 800, 480);
  delay(5000);

  Serial.println("Refreshing image...5");
  displayImage(SD, IMAGE_5, 800, 480);
  delay(5000);
}

// SD card initialization
/** @brief Mount the SD card and report its type and capacity. @return 0 on success, -1 on failure. */
int SD_init()
{
  if (!SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS))
  {
    Serial.println(F("ERROR: SPI bus initialization failed!"));
    return 1;
  }
  if (!SD.begin(SD_CS, SD_SPI, SD_SPI_FREQ))
  {
    Serial.println(F("ERROR: File system mount failed!"));
    SD_SPI.end();
    return 1;
  }
  else
  {
    Serial.println("Card Mount Successed");
    Serial.printf("SD Size: %lluMB \n", SD.cardSize() / (1024 * 1024));
    // hspi->end();
  }
  listDir(SD, "/", 2);
  Serial.println("**** TF Card init finished ****.");
  return 0;
}

/*
Function: List files and subdirectories in the specified directory.
Parameters:
  - fs: The file system object used to operate on the file system.
  - dirname: A pointer to the name of the directory whose contents are to be listed.
  - levels: Specifies the depth of recursive listing of subdirectories.
*/
/** @brief Print a directory tree for diagnostics after the card is mounted. */
void listDir(fs::FS & fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname); 
  File root = fs.open(dirname); // Open the specified directory.
  // If the directory cannot be opened.
  if (!root) { 
    Serial.println("Failed to open directory"); 
    return; 
  }
  // If the opened item is not a directory.
  if (!root.isDirectory()) { 
    Serial.println("Not a directory"); 
    return; 
  }
  File file = root.openNextFile(); // Open the next file in the directory.
  // While there are still files.
  while (file) { 
    // If the current file is a directory.
    if (file.isDirectory()) { 
      Serial.print("  DIR : "); 
      Serial.println(file.name());    // Print the directory name.
      // If there are still levels of depth for recursion.
      if (levels) { 
        listDir(fs, file.name(), levels - 1); // Recursively call the function to list the contents of the subdirectory, reducing the depth by 1.
      }
    } 
    // If the current file is not a directory, i.e., a regular file.
    else { 
      Serial.print("  FILE: "); 
      Serial.print(file.name());      // Print the file name.
      Serial.print("  SIZE: "); 
      Serial.println(file.size());    // Print the file size.
    }
    file = root.openNextFile();       // Continue to open the next file in the directory.
  }
}

/*
  Function:
    - Display image from file
  Parameters:
    - fs: The file system object
    - filename: The name of the BMP image file
    - x: The x - coordinate on the screen to start printing the image
    - y: The y - coordinate on the screen to start printing the image
  Returns:
    - 0 if the image is printed successfully, 0 if the file cannot be opened
*/
/** @brief Decode one BMP file into the LCD frame buffer. @return 0 on success, -1 on failure. */
int displayImage(fs::FS &fs, const char *filename, int x, int y)
{
    File f = fs.open(filename, "r"); // Open the file for reading
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;       // File open failed, return 0
    }
    int X = x;          // representing the width of the image
    int Y = y;          // representing the height of the image
    const size_t line_bytes = 3 * X;
    if (line_bytes > sizeof(bmp_line))
    {
        Serial.println("BMP line buffer is too small");
        f.close();
        return 0;
    }

    if (frame_ready) {
        frame.setBuffer(gfx._bus_instance.getBackBuffer(), X, Y, 16);
        frame.fillScreen(TFT_BLACK);
    } else {
        gfx.startWrite();
    }

    for (int row = 0; row < Y; row++) // Iterate through each row of the image
    {
        f.seek(54 + 3 * X * row);     // Jump to the position of the current row
        if (f.read(bmp_line, line_bytes) != line_bytes)
        {
            Serial.println("BMP read error");
            if (!frame_ready) {
                gfx.endWrite();
            }
            f.close();
            return 0;
        }
        if (frame_ready) {
            frame.pushImage(0, row, X, 1, (lgfx::rgb888_t *)bmp_line);
        } else {
            gfx.pushImage(0, row, X, 1, (lgfx::rgb888_t *)bmp_line);
        }
    }
    if (frame_ready) {
        gfx._bus_instance.present();
    } else {
        gfx.endWrite();
        gfx.waitDisplay();
    }
    f.close(); // Close the file
    return 0; 
}
