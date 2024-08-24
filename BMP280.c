#include "bmp280.h"
#include <stdlib.h>
#include <pico/stdlib.h>
#include "hardware/i2c.h"
#include <stdio.h>

// Define I2C instance and address
static i2c_inst_t *i2c_instance;
static uint8_t i2c_addr;

static void bmp280_write_reg(const uint8_t reg, const uint32_t size, const uint8_t* src) {
    uint8_t* buff = (uint8_t*)calloc(size + 1, 1);
    if (buff == NULL) {
        printf("Memory allocation failed in bmp280_write_reg\n");
        return; // Error handling
    }
    buff[0] = reg;
    for (uint32_t i = 0; i < size; i++) {
        buff[i + 1] = src[i];
    }
    int result = i2c_write_blocking(i2c_instance, i2c_addr, buff, size + 1, false);
    if (result < 0) {
        printf("I2C write failed in bmp280_write_reg\n");
    }
    free(buff);
}

static void bmp280_read_reg(const uint8_t reg, const uint32_t size, uint8_t* dst) {
    
    sleep_ms(20);
    int result = i2c_write_blocking(i2c_instance, i2c_addr, &reg, 1, false);
    if (result < 0) {
        printf("I2C write failed in bmp280_read_reg\n");
        return;
    }
    sleep_ms(20);

    result = i2c_read_blocking(i2c_instance, i2c_addr, dst, size, false);
    if (result < 0) {
        printf("I2C read failed in bmp280_read_reg\n");
    }
}

int bmp280_init(i2c_inst_t *i2c_instance_param, uint8_t i2c_addr_param) {
    i2c_instance = i2c_instance_param;
    i2c_addr = i2c_addr_param;

    sleep_ms(20);
    printf("BMP280 connected, initializing...\n");

    uint8_t chip_ID;
    bmp280_read_reg(BMP280_CHIP_ID_REG, 1, &chip_ID);
    if (chip_ID != BMP280_CHIP_ID) {
        printf("BMP280 chip ID mismatch: expected 0x%02x, got 0x%02x\n", BMP280_CHIP_ID, chip_ID);
        return -1;
    }
    
    // Reset registers
    uint8_t reset_val = BMP280_RESET_VAL;
    bmp280_write_reg(BMP280_RESET_REG, 1, &reset_val);
    sleep_ms(10);

    // Power control
    uint8_t ctl_data = (((BMP280_OVERSCAN_X2 << 3) | BMP280_OVERSCAN_X16) << 2) | BMP280_MODE_NORMAL;
    bmp280_write_reg(BMP280_POWER_CTL_REG, 1, &ctl_data);
    sleep_ms(10);

    return 1;
}

void bmp280_calibrate(bmp280* device) {
    bmp280_read_reg(0x88, 24, device->coefficients); // Calibration data start register

    // Extract calibration coefficients
    device->dig_T1 = (device->coefficients[1] << 8) | device->coefficients[0];
    device->dig_T2 = (device->coefficients[3] << 8) | device->coefficients[2];
    device->dig_T3 = (device->coefficients[5] << 8) | device->coefficients[4];
    device->dig_P1 = (device->coefficients[7] << 8) | device->coefficients[6];
    device->dig_P2 = (device->coefficients[9] << 8) | device->coefficients[8];
    device->dig_P3 = (device->coefficients[11] << 8) | device->coefficients[10];
    device->dig_P4 = (device->coefficients[13] << 8) | device->coefficients[12];
    device->dig_P5 = (device->coefficients[15] << 8) | device->coefficients[14];
    device->dig_P6 = (device->coefficients[17] << 8) | device->coefficients[16];
    device->dig_P7 = (device->coefficients[19] << 8) | device->coefficients[18];
    device->dig_P8 = (device->coefficients[21] << 8) | device->coefficients[20];
    device->dig_P9 = (device->coefficients[23] << 8) | device->coefficients[22];
}

// From Bosh documentation
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value

void bmp280_read_temperature(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_TEMPERATURE_REG_LOW, 3, data);

    int32_t adc_t = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int32_t var1, var2;
    var1 = ((((adc_t >> 3) - ((int32_t)device->dig_T1 << 1)) * 
        (int32_t)device->dig_T2) >> 11);
    var2 = (((((adc_t >> 4) - (int32_t)device->dig_T1) * ((adc_t >> 4) - 
        (int32_t)device->dig_T1)) >> 12) * (int32_t)device->dig_T3) >> 14;
    device->t_fine = var1 + var2;
    device->temperature = (device->t_fine * 5 + 128) >> 8;
}

// from Bosh documentation:
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa

void bmp280_read_pressure(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_PRESSURE_REG_LOW, 3, data);

    int32_t adc_p = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int64_t var1, var2;
    int64_t p;
    var1 = ((int64_t)device->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)device->dig_P6;
    var2 = var2 + ((var1 * (int64_t)device->dig_P5) << 17);
    var2 = var2 + ((int64_t)device->dig_P4 << 35);
    var1 = ((var1 * var1 * (int64_t)device->dig_P3) >> 8) + ((var1 * (int64_t)device->dig_P2) << 12);
    var1 = ((((int64_t)1 << 47) + var1)) * ((int64_t)device->dig_P1) >> 33;

    if (var1 == 0) {
        device->pressure = 0; // Avoid division by zero
        return;
    }

    p = 1048576 - adc_p;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)device->dig_P9 * (p << 31) * (p << 31)) >> 25;
    var2 = ((int64_t)device->dig_P8 * p) >> 19;
    device->pressure = ((p + var1 + var2) >> 8) + (((int64_t)device->dig_P7) << 4); 
}