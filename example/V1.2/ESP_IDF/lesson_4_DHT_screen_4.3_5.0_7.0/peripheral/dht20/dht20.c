/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include "dht20.h"
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
#ifdef FIR_FILTER_ENABLE
FIRFilter dht20_avg_temp;
FIRFilter dht20_avg_humid;
#endif
/*————————————————————————————————————————Variable declaration end———————————————————————————————————————*/

/*—————————————————————————————————————————Functional function———————————————————————————————————————————*/

/**
 * @brief  If returned sttaus is not ok reset register.
 * as per manufacturers datasheet and example.
 * @param register that should reset
 * @return true or false if reset was success or not.
 */
static bool dht20_reset_register(uint8_t reg)
{
    esp_err_t err = ESP_OK;
    static uint8_t values[3] = {0};
    uint8_t txbuffer[3] = { reg, 0x00, 0x00 };
    err = i2c_write(DHT20_I2C_ADDRESS, txbuffer, 3);            // write some zero's to register %^&#$();;; kaboom.
    if(err!=ESP_OK) 
    {
        return false;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
    i2c_read(DHT20_I2C_ADDRESS, values, 3);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    memset(txbuffer, 0, sizeof(txbuffer));                      // reset buffer before reuse
    txbuffer[0] = (0xB0 | reg);                                 // write again with some black magic applied.
    txbuffer[1] = values[1];
    txbuffer[2] = values[2];
    err = i2c_write(DHT20_I2C_ADDRESS, txbuffer, 3);
    if(err != ESP_OK) 
    {
        return false;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
    return true;
}

/**
 * @brief Read complete status byte
 * @param none
 * @return status byte
 */
uint8_t dht20_status(void)
{
    // static uint8_t txbuf[3]={0x71};
    static uint8_t txbuf = 0x71;
    static uint8_t rxdata;
    i2c_write(DHT20_I2C_ADDRESS, &txbuf, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    i2c_read(DHT20_I2C_ADDRESS, &rxdata, 1);
    return (rxdata);
}

/**
 * @brief Reset all 3 sensor registers as per datasheet/sample program.
 * @param   none
 * @return  reset counter, how many times a reset was need before status byte was 0x18.
 */
static uint8_t dht20_reset_sensor(void)
{
    static uint8_t rst_count = 255;
    uint8_t status = dht20_status();
    DHT20_DEBUG("Sensor status: %s - 0x%02X", print_byte(status), status);
    while((status&0x18)!=0x18) 
    {
        DHT20_DEBUG("Sensor status: %s - 0x%02X", print_byte(status), status);
        rst_count++;
        if(dht20_reset_register(0x1B)) 
        {
            rst_count++;
        }
        if(dht20_reset_register(0x1C)) 
        {
            rst_count++;
        }
        if(dht20_reset_register(0x1E)) 
        {
            rst_count++;
        }
        DHT20_DEBUG("Registers resetted [%d] times!", rst_count);
        status = dht20_status();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
    return rst_count;
}

/**
 * @brief Initialize I2C bus and sensor.
 * Status register is checked for code 0x18 if not the sensor will be reset.
 * @param none
 */
void dht20_begin(void)
{
    static esp_err_t err = ESP_OK;
    err = i2c_init();
    if (err != ESP_OK) {
        DHT20_ERROR("i2c init: [ %s ]", esp_err_to_name(err));
    }
    vTaskDelay(200/portTICK_PERIOD_MS);
    dht20_reset_sensor();  
// Check status register, if status not eq 0x18 reset the necearry registers.
#ifdef FIR_FILTER_ENABLE
    FIRFilter_Init(&dht20_avg_temp);
    FIRFilter_Init(&dht20_avg_humid);
#endif
}


/**
 * @brief Check the calibrated bit
 * @param  none
 * @return true = calibrated.
 */
bool dht20_is_calibrated(void)
{
    uint8_t status_byte = dht20_status();
    if((status_byte&0x18) == 0x18) 
    {
        return true;
    }
    return false;
}


/**
 * @brief  Calculate CRC check
 * @param array with all data
 * @param Num amount of bytes stored in array
 * @return crc check value.
 */
static uint8_t dht20_crc8(uint8_t *message, uint8_t Num)
{
    static uint8_t i;
    static uint8_t byte;
    uint8_t crc = 0xFF;
    for (byte = 0; byte < Num; byte++) {
        crc ^= (message[byte]);
        for(i = 8; i > 0; --i) 
        {
            if(crc & 0x80) 
            {
                crc = (crc << 1) ^ 0x31;
            } 
            else 
            {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/**
 * @brief Read full raw Temperature and huidity data from sensor.
 * Read status byte to see if sensor is still busy measuring.
 * Of ready read all 7 bytes.
 * Calculate CRC and compare value with CRC data byte rxdata[6] (which is byte 7 :-).
 * Convert data and update struct.
 * @param dht20_data_t struct
 * @return void
 */
float dht20_read_data(dht20_data_t *data)
{
    // zero struct values.
    data->humidity = 0.0f;
    data->raw_humid = 0;
    data->temperature = 0.0f;
    data->raw_temp = 0;

    static uint8_t txbuf[3] = {0xAC, 0x33, 0x00};               // Start measuring command + 2 bytes as parameters ?
    static uint8_t status_byte[1] = { 0 };                      // retrieve measuring status
    static uint8_t rxdata[7] = {0};                             // retrieve measurement data (temp + humid)

    i2c_write(DHT20_I2C_ADDRESS, txbuf, 3);
    vTaskDelay( 80 / portTICK_PERIOD_MS);                       // minimum delay as per datahseet.
    DHT20_DEBUG(" Reading registers.....");

    i2c_read(DHT20_I2C_ADDRESS, status_byte, 1);                // read status byte to check sensor is done measuring.

    unsigned long start_time = esp_timer_get_time() / 1000;
    while((status_byte[0] >> 7) != 0)// while meassurment is ongoing
    {                      
        if((esp_timer_get_time() / 1000) - start_time >= DHT20_MEASURE_TIMEOUT) {
            return DHT20_ERROR_READ_TIMEOUT;                    // Timeout
        }
        portYIELD();                                            // give scheduler some time for other tasks while we're still measuring.
    }

    i2c_read(DHT20_I2C_ADDRESS, rxdata, 7);                     // sensor idle let's read all data.
    DHT20_DEBUG("Byte1: %s", print_byte(rxdata[0]));
    DHT20_DEBUG("Byte2: %s", print_byte(rxdata[1]));
    DHT20_DEBUG("Byte3: %s", print_byte(rxdata[2]));
    DHT20_DEBUG("Byte4: %s", print_byte(rxdata[3]));
    DHT20_DEBUG("Byte5: %s", print_byte(rxdata[4]));
    DHT20_DEBUG("Byte6: %s", print_byte(rxdata[5]));
    DHT20_DEBUG("CRC Byte: %s", print_byte(rxdata[6]));

    uint8_t get_crc = dht20_crc8(rxdata, 6);                     // CRC calculation
    DHT20_DEBUG("Data byte 7: 0x%02X, calculated crc8: 0x%02X", rxdata[6], get_crc);

    if(rxdata[6] == get_crc) // Compare CRC calues and continue if they match.
    {                                 
        uint32_t raw_humid = rxdata[1];
        raw_humid <<= 8;
        raw_humid += rxdata[2];
        raw_humid <<= 4;
        raw_humid += rxdata[3] >> 4;
        data->raw_humid = raw_humid;
        data->humidity = (float)(raw_humid / 1048576.0f) * 100.0f;                                      // convert RAW to Humidity in %

        DHT20_DEBUG("Humidity raw: %lu - Converted: %.1f %%", data->raw_humid, data->humidity);

        uint32_t raw_temp = (rxdata[3] & 0x0F);
        raw_temp <<= 8;
        raw_temp += rxdata[4];
        raw_temp <<= 8;
        raw_temp += rxdata[5];
        data->raw_temp = raw_temp;
        data->temperature = (float)(raw_temp / 1048576.0f) * 200.0f - 50.0f;                            // convert RAW to Celsius C.

        DHT20_DEBUG("Temperature raw: %lu - Converted: %.2fC.", data->raw_temp, data->temperature);

#ifdef FIR_FILTER_ENABLE
    data->humid_avg = FIRFilter_Update(&dht20_avg_humid, data->humidity);
    data->temp_avg = FIRFilter_Update(&dht20_avg_temp, data->temperature);
#endif
    } 
    else 
    {
        DHT20_ERROR("CRC Checksum failed !!!");
        return DHT20_ERROR_CHECKSUM;
    }
    return 0.0f;
}

/*———————————————————————————————————————Functional function end—————————————————————————————————————————*/