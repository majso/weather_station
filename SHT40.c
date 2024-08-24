#include "sht40.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include "hardware/i2c.h"
#include <stdio.h>

// Define I2C instance and address
static i2c_inst_t *i2c_instance;
static uint8_t i2c_addr;

// Soft reset the SHT40 sensor
bool sht40_soft_reset() {
    uint8_t cmd[2] = {SHT40_SOFT_RESET >> 8, SHT40_SOFT_RESET & 0xFF}; // Using defined constant
    // Send soft reset command
    if (i2c_write_blocking(i2c_instance, i2c_addr, cmd, 2, false) < 0) {
        printf("SHT40 soft reset failed\n");
        return false;
    }

    sleep_ms(10); // Wait for reset to complete
    printf("SHT40 soft reset completed\n");
    return true;
}

// Initialize SHT40 sensor
void sht40_init(i2c_inst_t *i2c_instance_param, uint8_t i2c_addr_param) {
    i2c_instance = i2c_instance_param;
    i2c_addr = i2c_addr_param;

    printf("SHT40 connected, initializing...\n");

    // Perform a soft reset (optional, depending on your requirements)
    sht40_soft_reset();
}

// Read temperature and humidity data from the SHT40 sensor
bool sht40_read_data(float *temperature, float *humidity) {
    uint8_t cmd[2] = {SHT40_MEASURE_HIGHREP_STRETCH >> 8, SHT40_MEASURE_HIGHREP_STRETCH & 0xFF}; 
    uint8_t data[6];
    
    if (i2c_write_blocking(i2c_instance, i2c_addr, cmd, 2, false) < 0) {
        printf("SHT40 command send failed\n");
        return false;
    }

    // Wait for measurement to complete
    sleep_ms(50); // Adjust timing if needed

    if (i2c_read_blocking(i2c_instance, i2c_addr, data, sizeof(data), false) != sizeof(data)) {
        printf("SHT40 read data failed\n");
        return false;
    }

    uint16_t raw_temperature = (data[0] << 8) | data[1];
    uint16_t raw_humidity = (data[3] << 8) | data[4];

    *temperature = -45.0f + 175.0f * (raw_temperature / 65535.0f);
    *humidity = 100.0f * (raw_humidity / 65535.0f);

    return true;
}