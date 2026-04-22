#ifndef _I2c_H_
#define _I2C_H_

/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <rom/ets_sys.h>
#include "esp_timer.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
#define I2C_TAG "I2C"
#define I2C_INFO(fmt, ...)   ESP_LOGI(I2C_TAG, fmt, ##__VA_ARGS__)
#define I2C_DEBUG(fmt, ...)  ESP_LOGD(I2C_TAG, fmt, ##__VA_ARGS__)
#define I2C_ERROR(fmt, ...)  ESP_LOGE(I2C_TAG, fmt, ##__VA_ARGS__)

#ifdef CONFIG_DEVICE_I2C_ENABLED
#define I2C_MASTER_PORT     CONFIG_I2C_PORT_NUM
#define I2C_GPIO_SCL        CONFIG_I2C_GPIO_SCL
#define I2C_GPIO_SDA        CONFIG_I2C_GPIO_SDA
#define I2C_BUS_SPEED       CONFIG_I2C_MASTER_FREQUENCY
#define I2C_CMD_TIMEOUT     CONFIG_I2C_BUS_TIMEOUT

#ifndef CONFIG_I2C_GPIO_SCL_PULLUP
#define I2C_GPIO_SCL_PULLUP 0
#else
#define I2C_GPIO_SCL_PULLUP 1
#endif

#ifndef CONFIG_I2C_GPIO_SDA_PULLUP
#define I2C_GPIO_SDA_PULLUP 0
#else
#define I2C_GPIO_SDA_PULLUP 1
#endif

#endif

esp_err_t i2c_init(void);
esp_err_t i2c_read(uint8_t i2c_addr, uint8_t *read_buffer, size_t read_size);
esp_err_t i2c_write(uint8_t i2c_addr, uint8_t *write_buffer, size_t write_size);
void i2c_write_read(uint8_t i2c_addr, uint8_t read_reg, uint8_t *read_buffer, size_t read_size, uint16_t delayms);

char *print_binary(uint16_t value);
char *print_byte(uint8_t byte);

/*———————————————————————————————————————Variable declaration end——————————————-—————————————————————————*/
#endif