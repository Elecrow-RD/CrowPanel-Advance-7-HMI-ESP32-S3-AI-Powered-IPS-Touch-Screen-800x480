#include "LovyanGFX_Driver.h"
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>

#define LCD_H_RES 800
#define LCD_V_RES 480

#define SD_MOSI 6
#define SD_MISO 4
#define SD_SCK 5
#define SD_CS 0 // The chip selector pin is not connected to IO

// Array of image file paths for loop display
const char* IMAGE_FILES[] = {
  "/1.bmp",
  "/2.bmp",
  "/3.bmp",
  "/4.bmp",
  "/5.bmp"
};

#define IMAGE_COUNT (sizeof(IMAGE_FILES) / sizeof(IMAGE_FILES[0]))

SPIClass SD_SPI = SPIClass(HSPI);
LGFX gfx;

// Display text on the screen
void show_text(const char* text, uint16_t color = TFT_RED) {
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextSize(3);
  gfx.setTextColor(color);
  gfx.setCursor((LCD_H_RES - gfx.textWidth(text)) / 2, LCD_V_RES / 2 - 15); // Center the text
  gfx.print(text); 
}

// List files and subdirectories under a given directory
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname); 
  File root = fs.open(dirname);
  
  if (!root) { 
    Serial.println("Failed to open directory"); 
    return; 
  }
  
  if (!root.isDirectory()) { 
    Serial.println("Not a directory"); 
    return; 
  }
  
  File file = root.openNextFile();
  while (file) { 
    if (file.isDirectory()) { 
      Serial.print("  DIR : "); 
      Serial.println(file.name());
      if (levels) { 
        listDir(fs, file.name(), levels - 1);
      }
    } else { 
      Serial.print("  FILE: "); 
      Serial.print(file.name());
      Serial.print("  SIZE: "); 
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

// Initialize SD card
bool SD_init() {
  Serial.println("Initializing SD card...");
  
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SD_SPI, 40000000)) {
    Serial.println(F("ERROR: File system mount failed!"));
    SD_SPI.end();
    return false;
  }
  
  Serial.println("Card Mount Succeeded");
  Serial.printf("SD Size: %lluMB\n", SD.cardSize() / (1024 * 1024));
  listDir(SD, "/", 2);
  Serial.println("**** SD Card init finished ****");
  return true;
}

/*
  Display BMP image
  Parameters:
    - fs: File system object
    - filename: BMP image file name
    - width: Image width
    - height: Image height
  Returns:
    - true: Successfully displayed image
    - false: Failed to display image
*/
bool displayImage(fs::FS &fs, const char* filename, int width, int height) {
  Serial.printf("Opening image: %s\n", filename);
  
  File f = fs.open(filename, "r");
  if (!f) {
    Serial.printf("Failed to open file: %s\n", filename);
    return false;
  }
  
  // Check if the file size is valid
  size_t fileSize = f.size();
  if (fileSize < 54 || fileSize < (54 + 3 * width * height)) {
    Serial.println("Invalid BMP file size");
    f.close();
    return false;
  }
  
  // Skip the BMP header
  f.seek(54);
  
  // Allocate memory for one row of pixels
  uint8_t* rowBuffer = (uint8_t*)malloc(3 * width);
  if (!rowBuffer) {
    Serial.println("Memory allocation failed");
    f.close();
    return false;
  }
  
  // Read and display image row by row
  for (int row = 0; row < height; row++) {
    f.seek(54 + 3 * width * row);
    if (f.read(rowBuffer, 3 * width) != 3 * width) {
      Serial.println("Error reading image data");
      free(rowBuffer);
      f.close();
      return false;
    }
    gfx.pushImage(0, row, width, 1, (lgfx::rgb888_t*)rowBuffer);
  }
  
  // Free resources
  free(rowBuffer);
  f.close();
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n----- Starting setup -----");

  // Initialize display
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);
  delay(500);

  // Initialize I2C and IO expander
  Wire.begin(15, 16);
  delay(50);


  // Turn on backlight: Send 0 to 0x30
  Wire.beginTransmission(0x30);
  Wire.write(0);
  Wire.endTransmission();

  // Initialize SD card
  if (SD_init()) {
    Serial.println("SD Card initialization succeeded");
    show_text("SD Card OK", TFT_GREEN);
  } else {
    Serial.println("SD Card initialization failed");
    show_text("SD Card Failed", TFT_RED);
  }
  
  delay(2000);
  gfx.setRotation(2);
  gfx.fillScreen(TFT_BLACK);
  Serial.println("----- Setup done -----");
}

void loop() {
  static int currentImage = 0;
  
  // Display current image
  Serial.printf("Displaying image %d: %s\n", currentImage + 1, IMAGE_FILES[currentImage]);
  
  if (!displayImage(SD, IMAGE_FILES[currentImage], LCD_H_RES, LCD_V_RES)) {
    // If display fails, show error message
    char errorMsg[50];
    sprintf(errorMsg, "Failed to load %s", IMAGE_FILES[currentImage]);
    show_text(errorMsg, TFT_RED);
  }
  
  // Move to next image
  currentImage = (currentImage + 1) % IMAGE_COUNT;
  
  delay(5000);
}
