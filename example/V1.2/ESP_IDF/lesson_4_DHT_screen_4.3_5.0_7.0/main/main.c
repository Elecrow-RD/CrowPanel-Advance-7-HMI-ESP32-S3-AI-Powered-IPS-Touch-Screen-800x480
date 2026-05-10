/* SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

 #include "waveshare_rgb_lcd_port.h"
 #include "ui.h"
 #include "driver/gpio.h"
 #include "driver/i2c.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/semphr.h"
 #include "main.h"
 #include <stdio.h>
 
 /* ----------------------------- Macro Definitions ----------------------------- */
 #define I2C_MASTER_SDA_IO    15
 #define I2C_MASTER_SCL_IO    16
 #define I2C_MASTER_FREQ_HZ   400000
 #define I2C_MASTER_PORT      I2C_NUM_1
 #define TCA9534_ADDR         0x18
 
 /* ----------------------------- Global Variables ----------------------------- */
 static SemaphoreHandle_t data_mutex = NULL;
 static struct {
     float temperature;
     float humidity;
 } sensor_data;
 
 lv_obj_t *label_big_temp;
 lv_obj_t *label_big_humid;
 
 /* ------------------------- I2C Functions ------------------------- */
 void i2c_master_init(void) {
     i2c_config_t config = {
         .mode = I2C_MODE_MASTER,
         .sda_io_num = I2C_MASTER_SDA_IO,
         .scl_io_num = I2C_MASTER_SCL_IO,
         .sda_pullup_en = GPIO_PULLUP_ENABLE,
         .scl_pullup_en = GPIO_PULLUP_ENABLE,
         .master.clk_speed = I2C_MASTER_FREQ_HZ
     };
     i2c_param_config(I2C_MASTER_PORT, &config);
     i2c_driver_install(I2C_MASTER_PORT, config.mode, 0, 0, 0);
 }
 
 esp_err_t i2c_write_register(uint8_t reg_addr, uint8_t data) {
     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
     i2c_master_start(cmd);
     i2c_master_write_byte(cmd, (TCA9534_ADDR << 1) | I2C_MASTER_WRITE, true);
     i2c_master_write_byte(cmd, reg_addr, true);
     i2c_master_write_byte(cmd, data, true);
     i2c_master_stop(cmd);
     esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_PERIOD_MS);
     i2c_cmd_link_delete(cmd);
     return ret;
 }
 
 esp_err_t i2c_read_register(uint8_t reg_addr, uint8_t *data) {
     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
     i2c_master_start(cmd);
     i2c_master_write_byte(cmd, (TCA9534_ADDR << 1) | I2C_MASTER_WRITE, true);
     i2c_master_write_byte(cmd, reg_addr, true);
     i2c_master_start(cmd);
     i2c_master_write_byte(cmd, (TCA9534_ADDR << 1) | I2C_MASTER_READ, true);
     i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
     i2c_master_stop(cmd);
     esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_PERIOD_MS);
     i2c_cmd_link_delete(cmd);
     return ret;
 }
 
 // Write a byte to an address
esp_err_t i2c_write_byte(uint8_t device_addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}
 
 /* ------------------------- UI Initialization ------------------------- */
 void init_big_display(void) {
     label_big_temp = lv_label_create(lv_scr_act());
     lv_obj_set_style_text_font(label_big_temp, &lv_font_montserrat_48, 0);
     lv_obj_set_style_text_color(label_big_temp, lv_color_hex(0x000000), 0);
     lv_obj_align(label_big_temp, LV_ALIGN_TOP_LEFT, 310, 110);
 
     label_big_humid = lv_label_create(lv_scr_act());
     lv_obj_set_style_text_font(label_big_humid, &lv_font_montserrat_48, 0);
     lv_obj_set_style_text_color(label_big_humid, lv_color_hex(0x000000), 0);
     lv_obj_align(label_big_humid, LV_ALIGN_TOP_LEFT, 310, 260);
 }
 
 /* ------------------------- Display Update Task ------------------------- */
 void update_display_task(void *arg) {
     char temp_str[8];
     char humid_str[8];
 
     while (1) {
         if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
             snprintf(temp_str, sizeof(temp_str), "%.1f", sensor_data.temperature);
             snprintf(humid_str, sizeof(humid_str), "%.1f", sensor_data.humidity);
 
             if (lvgl_port_lock(-1)) {
                 lv_label_set_text(label_big_temp, temp_str);
                 lv_label_set_text(label_big_humid, humid_str);
                 lvgl_port_unlock();
             }
 
             xSemaphoreGive(data_mutex);
         }
         vTaskDelay(pdMS_TO_TICKS(200));
     }
 }
 
 /* ------------------------- Sensor Reading Task ------------------------- */
 void dht20_read_task(void *param) {
     dht20_data_t measurements;
 
     while (1) {
         if (dht20_read_data(&measurements) == ESP_OK) {
             xSemaphoreTake(data_mutex, portMAX_DELAY);
             sensor_data.temperature = measurements.temperature;
             sensor_data.humidity = measurements.humidity;
             xSemaphoreGive(data_mutex);
         }
         vTaskDelay(pdMS_TO_TICKS(1500));
     }
 }
 
 /* ------------------------- Main Function ------------------------- */
 void app_main(void) {
     // Initialize I2C and configure TCA9534 output
     i2c_master_init();
     vTaskDelay(pdMS_TO_TICKS(50));
 
     i2c_write_byte(0x30, 0x18);

     i2c_write_byte(0x30, 0x10);
 
     // GPIO Initialization
     gpio_config_t io_conf = {
         .pin_bit_mask = (1ULL << GPIO_NUM_1),
         .mode = GPIO_MODE_OUTPUT,
     };
     gpio_config(&io_conf);
     gpio_set_level(GPIO_NUM_1, 0);
     vTaskDelay(pdMS_TO_TICKS(20));
     gpio_set_level(GPIO_NUM_1, 1);
 
     // Initialize LCD and UI
     waveshare_esp32_s3_rgb_lcd_init();
 
     lvgl_port_lock(-1);
     ui_init();
     init_big_display();
     lvgl_port_unlock();
 
     // Synchronization mutex
     data_mutex = xSemaphoreCreateMutex();
 
     // Start tasks
     xTaskCreatePinnedToCore(dht20_read_task, "sensor", 4096, NULL, 5, NULL, 1);
     xTaskCreatePinnedToCore(update_display_task, "display", 4096, NULL, 4, NULL, 1);
 }
 