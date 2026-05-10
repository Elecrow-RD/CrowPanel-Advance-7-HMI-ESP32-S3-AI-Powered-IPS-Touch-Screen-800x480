#ifndef _DHT20_H_
#define _DHT20_H_

/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "i2c.h"
#ifdef CONFIG_FIR_FILTER_ENABLE
#define FIR_FILTER_ENABLE 1
#else
#define FIR_FILTER_ENABLE 0
#endif

#ifdef FIR_FILTER_ENABLE
#include "firfilter.h"
#endif
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
#define DHT20_TAG "DHT20"
#define DHT20_INFO(fmt, ...)   ESP_LOGI(DHT20_TAG, fmt, ##__VA_ARGS__)
#define DHT20_DEBUG(fmt, ...)  ESP_LOGD(DHT20_TAG, fmt, ##__VA_ARGS__)
#define DHT20_ERROR(fmt, ...)  ESP_LOGE(DHT20_TAG, fmt, ##__VA_ARGS__)

#define DHT20_I2C_ADDRESS      CONFIG_I2C_ADDRESS
#define DHT20_MEASURE_TIMEOUT   1000
typedef struct dht20_data {
    float temperature;
    float temp_avg;
    float humidity;
    float humid_avg;
    uint32_t raw_humid;
    uint32_t raw_temp;
} dht20_data_t;

typedef enum {
    DHT20_ERROR_READ_TIMEOUT = -549,
    DHT20_ERROR_CHECKSUM,
} dht20_error_t;


void dht20_begin(void);
bool dht20_is_calibrated(void);
float dht20_read_data(dht20_data_t *data);
/*———————————————————————————————————————Variable declaration end——————————————-—————————————————————————*/
#endif