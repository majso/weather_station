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
    printf("BMP280 chip ID match: expected 0x%02x, got 0x%02x\n", BMP280_CHIP_ID, chip_ID);
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
}

void bmp280_read_pressure(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_PRESSURE_REG_LOW, 3, data);

    int32_t adc_p = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int32_t var1, var2; 
    int64_t p;
    int64_t dig_P1 = (device->coefficients[7] << 8) | device->coefficients[6];
    int64_t dig_P2 = (device->coefficients[9] << 8) | device->coefficients[8];
    int64_t dig_P3 = (device->coefficients[11] << 8) | device->coefficients[10];
    int64_t dig_P4 = (device->coefficients[13] << 8) | device->coefficients[12];
    int64_t dig_P5 = (device->coefficients[15] << 8) | device->coefficients[14];
    int64_t dig_P6 = (device->coefficients[17] << 8) | device->coefficients[16];
    int64_t dig_P7 = (device->coefficients[19] << 8) | device->coefficients[18];
    int64_t dig_P8 = (device->coefficients[21] << 8) | device->coefficients[20];
    int64_t dig_P9 = (device->coefficients[23] << 8) | device->coefficients[22];

    var1 = (device->temperature * 5120.0) - 128000;
    var2 = var1 * var1 * dig_P6;
    var2 += ((var1 * dig_P5) << 17);
    var2 += (dig_P4 << 35);
    var1 = ((var1 * var1 * dig_P3) >> 8) + ((var1 * dig_P2) << 12);
    var1 = ((((int64_t)1 << 47) + var1) * dig_P1) >> 33;

    if (var1 == 0) {
        device->pressure = 0; // Avoid division by zero
        return;
    }

    p = 1048576 - adc_p;
    p = ((p - (var2 >> 12)) * 3125) / var1;
    var1 = (dig_P9 * p * p) >> 13;
    var2 = (dig_P8 * p) >> 13;
    device->pressure = (p + ((var1 + var2 + dig_P7) >> 4)); 
}

void bmp280_read_temperature(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_TEMPERATURE_REG_LOW, 3, data);

    int32_t adc_t = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int32_t var1, var2;
    int32_t dig_T1 = (device->coefficients[1] << 8) | device->coefficients[0];
    int32_t dig_T2 = (device->coefficients[3] << 8) | device->coefficients[2];
    int32_t dig_T3 = (device->coefficients[5] << 8) | device->coefficients[4];

    var1 = ((adc_t / 16384.0) - (dig_T1 / 1024.0)) * dig_T2;
    var2 = ((adc_t / 131072.0) - (dig_T1 / 8192.0)) * (adc_t / 131072.0) * dig_T3;
    device->temperature = (var1 + var2) / 5120.0;
}