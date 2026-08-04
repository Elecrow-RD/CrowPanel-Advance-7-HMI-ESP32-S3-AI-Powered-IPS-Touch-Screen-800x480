#include "pins_config.h"
#include "LovyanGFX_Driver.h"

#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>

#include <stdbool.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "ui.h"

LGFX gfx;

static uint8_t *buf;
static uint8_t *buf1;

constexpr uint32_t LVGL_BUFFER_LINES = 48;

uint16_t touch_x, touch_y;

//  Display refresh
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1,
                   area->y2 - area->y1 + 1,
                   reinterpret_cast<lgfx::rgb565_t *>(px_map));
  gfx.waitDMA();

  lv_display_flush_ready(disp);
}

//  Read touch
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  data->state = LV_INDEV_STATE_REL;// The state of data existence when releasing the finger
  bool touched = gfx.getTouch( &touch_x, &touch_y );
  if (touched)
  {
    data->state = LV_INDEV_STATE_PR;

    //  Set coordinates
    data->point.x = touch_x;
    data->point.y = touch_y;
  }
}

uint32_t my_tick_get_cb()
{
  return millis();
}

bool i2cScanForAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// Wrapper function for sending I2C commands
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

void setup()
{
  Serial.begin(115200); 

  pinMode(19, OUTPUT);

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
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);

  lv_init();
  lv_tick_set_cb(my_tick_get_cb);

  constexpr size_t buffer_size = LCD_H_RES * LVGL_BUFFER_LINES * sizeof(uint16_t);
  buf = static_cast<uint8_t *>(heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  buf1 = static_cast<uint8_t *>(heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (buf == nullptr || buf1 == nullptr) {
    Serial.println("Failed to allocate LVGL display buffers");
    abort();
  }

  lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(display, my_disp_flush);
  lv_display_set_buffers(display, buf, buf1, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  delay(100);
  gfx.fillScreen(TFT_BLACK);
  // lv_demo_widgets();// Main UI interface
  ui_init();

  Serial.println( "Setup done" );
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(1);
}
