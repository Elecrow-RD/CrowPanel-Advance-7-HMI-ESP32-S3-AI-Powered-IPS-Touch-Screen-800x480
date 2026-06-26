/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include "i2c.h"
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
/*————————————————————————————————————————Variable declaration end———————————————————————————————————————*/

/*—————————————————————————————————————————Functional function———————————————————————————————————————————*/
esp_err_t i2c_init(void)
{
    static esp_err_t err = ESP_OK;
    int i2c_master_port = I2C_MASTER_PORT;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_GPIO_SDA,         
        .sda_pullup_en = I2C_GPIO_SDA_PULLUP,
        .scl_io_num = I2C_GPIO_SCL,         
        .scl_pullup_en = I2C_GPIO_SCL_PULLUP,
        .master.clk_speed = I2C_BUS_SPEED,  
        .clk_flags = 0,                         
    };
    I2C_INFO("Initializing I2C Bus.......");
    err = i2c_param_config(i2c_master_port, &conf);
    err += i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
    I2C_INFO("I2C Bus Initialized, %s", esp_err_to_name(err));
    return err;
}

esp_err_t i2c_read(uint8_t i2c_addr, uint8_t *read_buffer, size_t read_size)
{
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, i2c_addr << 1 | I2C_MASTER_READ, true);
    i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_PORT, handle, pdMS_TO_TICKS(I2C_CMD_TIMEOUT));

    i2c_cmd_link_delete(handle);

    return err;
}


static esp_err_t _i2c_write_read(uint8_t i2c_addr, uint8_t read_reg, uint8_t *read_buffer, size_t read_size, uint16_t delayms)
{

    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    assert (handle != NULL);

    i2c_master_start(handle);
    i2c_master_write_byte(handle, i2c_addr << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, &read_reg, 1, true);
    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_PORT, handle, pdMS_TO_TICKS(I2C_CMD_TIMEOUT));
    i2c_cmd_link_delete(handle);

    if (delayms > 0) {
        portDISABLE_INTERRUPTS();
        ets_delay_us(delayms * 1000);
        portENABLE_INTERRUPTS();
    }

    handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, i2c_addr << 1 | I2C_MASTER_READ, true);
    i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);
    err += i2c_master_cmd_begin(I2C_MASTER_PORT, handle, pdMS_TO_TICKS(I2C_CMD_TIMEOUT));

    i2c_cmd_link_delete(handle);
    return err;
}

esp_err_t i2c_write(uint8_t i2c_addr, uint8_t *write_buffer, size_t write_size)
{
    esp_err_t err = ESP_OK;

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    assert (handle != NULL);

    i2c_master_start(handle);
    i2c_master_write_byte(handle, i2c_addr << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, write_buffer, write_size, true);

    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_PORT, handle, pdMS_TO_TICKS(I2C_CMD_TIMEOUT));

    i2c_cmd_link_delete(handle);
    return err;
}


void i2c_write_read(uint8_t i2c_addr, uint8_t read_reg, uint8_t *read_buffer, size_t read_size, uint16_t delayms)
{
    _i2c_write_read(i2c_addr, read_reg, read_buffer, read_size, delayms);
}

char *print_binary(uint16_t value)
{
    static char binary_str[17];  // 16 bits + null-terminator
    binary_str[16] = '\0'; // Null-terminate the string

    for (int i = 15; i >= 0; i--) {
        binary_str[15 - i] = ((value >> i) & 1) ? '1' : '0';
    }

    return binary_str;
}


const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

char *print_byte(uint8_t byte)
{
    static char binbyte[11];
    sprintf(binbyte, "0b%s %s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);

    return binbyte;
}

/*———————————————————————————————————————Functional function end—————————————————————————————————————————*/