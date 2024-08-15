#include "sht30.h"

// Define I2C instance and address
static i2c_inst_t *i2c_instance;
static uint8_t i2c_addr;

// Initialize SHT30 sensor
void sht30_init(i2c_inst_t *i2c_instance_param, uint8_t i2c_addr_param) {
    i2c_instance = i2c_instance_param;
    i2c_addr = i2c_addr_param;

    // Initialize I2C
    i2c_init(i2c_instance, 100 * 1000); // 100kHz
}

// Soft reset the SHT30 sensor
void sht30_soft_reset() {
    uint8_t cmd[2] = {0x30, 0xA2};
    i2c_write_blocking(i2c_instance, i2c_addr, cmd, 2, false);
    sleep_ms(10); // Wait for reset to complete
}

// Read temperature and humidity data from the SHT30 sensor
bool sht30_read_data(float *temperature, float *humidity) {
    uint8_t cmd[2] = {SHT30_MEASURE_HIGHREP_STRETCH >> 8, SHT30_MEASURE_HIGHREP_STRETCH & 0xFF};
    uint8_t data[6];

    // Send measurement command
    i2c_write_blocking(i2c_instance, i2c_addr, cmd, 2, false);

    // Wait for measurement to complete
    sleep_ms(50); // 50ms delay for high repetition rate

    // Read measurement data
    if (i2c_read_blocking(i2c_instance, i2c_addr, data, 6, false) != 6) {
        return false; // Read error
    }

    // Convert data to temperature and humidity
    uint16_t raw_temperature = (data[0] << 8) | data[1];
    uint16_t raw_humidity = (data[3] << 8) | data[4];

    *temperature = -45 + 175 * (raw_temperature / 65535.0);
    *humidity = 100 * (raw_humidity / 65535.0);

    return true;
}
