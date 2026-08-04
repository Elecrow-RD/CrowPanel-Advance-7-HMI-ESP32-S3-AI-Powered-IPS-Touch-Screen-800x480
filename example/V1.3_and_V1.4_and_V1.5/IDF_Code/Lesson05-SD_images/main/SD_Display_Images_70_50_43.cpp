#include "LovyanGFX_Driver.h"

#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <esp_heap_caps.h>

#define LCD_H_RES 800
#define LCD_V_RES 480

#define SD_MOSI 6
#define SD_MISO 4
#define SD_SCK  5
#define SD_CS   0

#define DISPLAY_HOLD_MS     5000
#define SD_READ_CHUNK_BYTES 32768

const char* IMAGE_FILES[] = {
  "/1.bmp",
  "/2.bmp",
  "/3.bmp",
  "/4.bmp",
  "/5.bmp"
};

#define IMAGE_COUNT (sizeof(IMAGE_FILES) / sizeof(IMAGE_FILES[0]))
#define IMAGE_PIXEL_BYTES ((size_t)LCD_H_RES * LCD_V_RES * 3)

static constexpr uint8_t kUiRotation = 0;
static constexpr uint8_t kImageRotation = 2;

SPIClass SD_SPI = SPIClass(HSPI);
LGFX gfx;

static uint8_t* imageSlots[IMAGE_COUNT] = {};
static int showIdx = 0;
static bool allImagesLoaded = false;

static void show_text(const char* text, uint16_t color = TFT_RED) {
  gfx.setRotation(kUiRotation);
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextSize(3);
  gfx.setTextColor(color);
  gfx.setCursor((LCD_H_RES - gfx.textWidth(text)) / 2, LCD_V_RES / 2 - 15);
  gfx.print(text);
  gfx.endWrite();
}

static bool allocImageSlots(void) {
  for (int i = 0; i < (int)IMAGE_COUNT; ++i) {
    if (imageSlots[i]) {
      continue;
    }
    imageSlots[i] = (uint8_t*)heap_caps_malloc(IMAGE_PIXEL_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!imageSlots[i]) {
      Serial.printf("Failed to allocate PSRAM for image %d\n", i + 1);
      return false;
    }
  }
  return true;
}

static bool loadBmpToBuffer(int imageIdx, uint8_t* dst) {
  File f = SD.open(IMAGE_FILES[imageIdx], "r");
  if (!f) {
    Serial.printf("Failed to open file: %s\n", IMAGE_FILES[imageIdx]);
    return false;
  }

  size_t fileSize = f.size();
  if (fileSize < 54 || fileSize < (54 + IMAGE_PIXEL_BYTES)) {
    Serial.println("Invalid BMP file size");
    f.close();
    return false;
  }

  f.seek(54);
  size_t offset = 0;
  while (offset < IMAGE_PIXEL_BYTES) {
    size_t chunk = IMAGE_PIXEL_BYTES - offset;
    if (chunk > SD_READ_CHUNK_BYTES) {
      chunk = SD_READ_CHUNK_BYTES;
    }
    if (f.read(dst + offset, chunk) != chunk) {
      Serial.println("Error reading image data");
      f.close();
      return false;
    }
    offset += chunk;
    delay(1);
  }
  f.close();
  return true;
}

static bool loadAllImagesFromSd(void) {
  if (!allocImageSlots()) {
    return false;
  }

  show_text("Loading images...", TFT_YELLOW);

  for (int i = 0; i < (int)IMAGE_COUNT; ++i) {
    Serial.printf("Loading image %d/%u: %s\n", i + 1, (unsigned)IMAGE_COUNT, IMAGE_FILES[i]);
    if (!loadBmpToBuffer(i, imageSlots[i])) {
      return false;
    }
  }
  return true;
}

static void presentImage(int imageIdx) {
  gfx.setRotation(kImageRotation);
  gfx.beginBackBufferDraw();
  gfx.startWrite();
  gfx.pushImage(0, 0, LCD_H_RES, LCD_V_RES, (lgfx::rgb888_t*)imageSlots[imageIdx]);
  gfx.endWrite();
  gfx.commitBackBuffer();
  gfx.setRotation(kUiRotation);
}

static void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
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

static bool SD_init(void) {
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SD_SPI, 40000000)) {
    Serial.println("SD mount failed");
    SD_SPI.end();
    return false;
  }
  Serial.printf("SD mounted, size %lluMB\n", SD.cardSize() / (1024 * 1024));
  listDir(SD, "/", 2);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n----- Starting setup -----");

  gfx.init();
  gfx.setRotation(kUiRotation);
  gfx.fillScreen(TFT_BLACK);
  delay(500);

  Wire.begin(15, 16);
  delay(50);
  Wire.beginTransmission(0x30);
  Wire.write(0);
  Wire.endTransmission();

  if (SD_init()) {
    Serial.println("SD Card initialization succeeded");
    show_text("SD Card OK", TFT_GREEN);
    delay(1000);
    allImagesLoaded = loadAllImagesFromSd();
    if (!allImagesLoaded) {
      show_text("Image load failed", TFT_RED);
    }
  } else {
    Serial.println("SD Card initialization failed");
    show_text("SD Card Failed", TFT_RED);
  }

  delay(500);
  gfx.fillScreen(TFT_BLACK);
  Serial.println("----- Setup done -----");
}

void loop() {
  if (!allImagesLoaded) {
    delay(2000);
    return;
  }

  Serial.printf("Presenting image %d: %s\n", showIdx + 1, IMAGE_FILES[showIdx]);
  presentImage(showIdx);
  showIdx = (showIdx + 1) % IMAGE_COUNT;
  delay(DISPLAY_HOLD_MS);
}
