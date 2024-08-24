#include "sht40.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include "hardware/i2c.h"
#include <stdio.h>

// Define I2C instance and address
static i2c_inst_t *i2c_instance;
static uint8_t i2c_addr;

// Initialize SHT40 sensor
void sht40_init(i2c_inst_t *i2c_instance_param, uint8_t i2c_addr_param) {
    i2c_instance = i2c_instance_param;
    i2c_addr = i2c_addr_param;

    printf("SHT40 connected, initializing...\n");

    // Perform a soft reset
    uint8_t soft_reset_cmd = SHT40_SOFT_RESET; // Soft reset command
    if (i2c_write_blocking(i2c_instance, i2c_addr, &soft_reset_cmd, sizeof(soft_reset_cmd), true) != PICO_ERROR_GENERIC) {
        printf("SHT40 initialized\n");
    } else {
        printf("SHT40 soft reset failed\n");
    }
}

// Read temperature and humidity data from the SHT40 sensor
bool sht40_read_data(float *temperature, float *humidity) {
    uint8_t buffer[6];
    
    // Send measurement command (e.g., high repeatability)
    uint8_t measure_cmd = SHT40_MEASURE_HIGHREP_STRETCH; // Measurement command
    if (i2c_write_blocking(i2c_instance, i2c_addr, &measure_cmd, sizeof(measure_cmd), false) < 0) {
        printf("SHT40 command send failed\n");
        return false;
    }

    // Wait for measurement to complete
    sleep_ms(50); // Adjust timing if needed

    // Read data from sensor
    if (i2c_read_blocking(i2c_instance, i2c_addr, buffer, sizeof(buffer), false) != sizeof(buffer)) {
        printf("SHT40 read data failed\n");
        return false;
    }

    uint16_t raw_temperature = ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1];
    uint16_t raw_humidity = ((uint16_t)buffer[3] << 8) | (uint16_t)buffer[4];

    *temperature = -45.0f + 175.0f * (raw_temperature / 65535.0f);
    *humidity = 100.0f * (raw_humidity / 65535.0f);

    return true;
}