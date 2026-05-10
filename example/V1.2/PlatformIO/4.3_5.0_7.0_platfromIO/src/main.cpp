#include "pins_config.h"
#include "LovyanGFX_Driver.h"

#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <Wire.h>

#include <stdbool.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "ui.h"


/* Expand IO */
#include <TCA9534.h>
TCA9534 ioex;

LGFX gfx;

/* Change to your screen resolution */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;
static lv_color_t *buf1;

uint16_t touch_x, touch_y;

//  Display refresh
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);

  lv_disp_flush_ready(disp);  //	Tell lvgl that the refresh is complete
}

bool i2cScanForAddress(uint8_t address)
{
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

//  Read touch
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
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

void scanI2C() {
  Serial.println("Scanning I2C...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
}

// 封装函数，用于发送 I2C 命令
void sendI2CCommand(uint8_t command) {
  uint8_t error;
  // 开始向指定地址发送命令
  Wire.beginTransmission(0x30);
  // 发送命令
  Wire.write(command);
  // 结束传输并返回状态
  error = Wire.endTransmission();

  if (error == 0) {
    Serial.print("命令 0x");
    Serial.print(command, HEX);
    Serial.println(" 发送成功");
  } else {
    Serial.print("命令发送错误，错误代码：");
    Serial.println(error);
  }
}

void setup()
{
  Serial.begin(115200); 

  // 初始化 PSRAM 并设置时钟
  #if CONFIG_SPIRAM_SUPPORT
    // 强制设置 PSRAM 时钟为 120MHz
    esp_psram_extram_set_clock_rate(120 * 1000000); 
    if (!psramInit()) {
      Serial.println("PSRAM 初始化失败！");
      while(1);  // 卡死以指示错误
    }
    Serial.println("PSRAM 初始化成功");
  #endif

  // 验证 PSRAM 大小
  Serial.printf("PSRAM 总大小: %d MB\n", ESP.getPsramSize() / 1024 / 1024);

  // pinMode(19, OUTPUT);//uart1

  // Wire.begin();
  // Wire.end();             // 释放当前总线
  // Wire.setPins(15, 16);    // 设置新引脚（SDA=GPIO5, SCL=GPIO6）
  // Wire.begin();           // 重新初始化总线

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
      //防止单片机没启动调节亮屏
      sendI2CCommand(0x19);
      pinMode(1, OUTPUT);
      digitalWrite(1, LOW);
      //ioex.output(2, TCA9534::Level::L);
      //ioex.output(2, TCA9534::Level::H);
      delay(120);
      pinMode(1, INPUT);

      delay(100);
    }
  }


  if (i2cScanForAddress(0x30) && i2cScanForAddress(0x5D)) // new V1.2
  {
    // Wire.beginTransmission(0x30);
    // Wire.write(0x10);
    // Wire.endTransmission();

    Wire.write(0x18);
    Wire.endTransmission();
  }
  else
  {
    ioex.attach(Wire);
    ioex.setDeviceAddress(0x18);
    ioex.config(1, TCA9534::Config::OUT);
    ioex.config(2, TCA9534::Config::OUT);

    /* Turn on backlight*/
    ioex.output(1, TCA9534::Level::H);

    delay(20);
    ioex.output(2, TCA9534::Level::H);
    delay(100);
    pinMode(1, INPUT);
    /*end*/
  }

  // Init Display
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);

  lv_init();
  // size_t buffer_size = sizeof(lv_color_t) * LCD_H_RES * LCD_V_RES;
  size_t buffer_size = sizeof(lv_color_t) * LCD_H_RES * LCD_V_RES;
  buf = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);

  lv_disp_draw_buf_init(&draw_buf, buf, buf1, LCD_H_RES * LCD_V_RES);

  // Initialize display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  // Change the following lines to your display resolution
  disp_drv.hor_res = LCD_H_RES;
  disp_drv.ver_res = LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize input device driver program
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  delay(100);
  gfx.fillScreen(TFT_BLACK);

  Wire.write(0x10);
  ui_init();

  Serial.println( "Setup done" );
}

// void print_psram_clk() {
//   uint32_t clk_speed;
//   esp_psram_extram_get_clock_rate(&clk_speed);
//   Serial.printf("PSRAM 实际时钟频率: %d MHz\n", clk_speed / 1000000);
// }

void loop()
{
  // print_psram_clk();
  lv_timer_handler(); /* let the GUI do its work */
  delay(1);
}
