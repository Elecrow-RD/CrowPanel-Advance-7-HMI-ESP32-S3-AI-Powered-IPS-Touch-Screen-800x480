#ifndef _RGB_LCD_H_
#define _RGB_LCD_H_

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"
#include "lv_demos.h"
#include "lvgl_port.h"

#define CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_GT911 1 // 1 initiates the touch, 0 closes the touch.

#define I2C_MASTER_SCL_IO           16       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           15       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0       /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define GPIO_INPUT_IO_4    4
#define GPIO_INPUT_PIN_SEL  1ULL<<GPIO_INPUT_IO_4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_H_RES               (LVGL_PORT_H_RES)
#define EXAMPLE_LCD_V_RES               (LVGL_PORT_V_RES)
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (16 * 1000 * 1000)
#define EXAMPLE_LCD_BIT_PER_PIXEL       (16)
#define EXAMPLE_RGB_BIT_PER_PIXEL       (16)
#define EXAMPLE_RGB_DATA_WIDTH          (16)
#define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * CONFIG_EXAMPLE_LCD_RGB_BOUNCE_BUFFER_HEIGHT)
#define EXAMPLE_LCD_IO_RGB_DISP         (-1)             // -1 if not used
#define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_41)
#define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_40)
#define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_42)
#define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_39)
#define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_21)
#define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_47)
#define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_48)
#define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_45)
#define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_38)
#define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_9)
#define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_10)
#define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_11)
#define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_12)
#define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_13)
#define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_14)
#define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_7)
#define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_17)
#define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_18)
#define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_3)
#define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_46)

#define EXAMPLE_LCD_IO_RST              (-1)             // -1 if not used
#define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)    // -1 if not used
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

#define EXAMPLE_PIN_NUM_TOUCH_RST       (-1)            // -1 if not used
#define EXAMPLE_PIN_NUM_TOUCH_INT       (-1)            // -1 if not used

static const char *TAG = "example";

bool example_lvgl_lock(int timeout_ms);
void example_lvgl_unlock(void);

esp_err_t waveshare_esp32_s3_rgb_lcd_init();

esp_err_t wavesahre_rgb_lcd_bl_on();
esp_err_t wavesahre_rgb_lcd_bl_off();

void example_lvgl_demo_ui();

#endif