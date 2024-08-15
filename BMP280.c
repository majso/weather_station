#include "bmp280.h"
#include <stdlib.h>
#include <pico/stdlib.h>
#include "hardware/i2c.h"

static void bmp280_write_reg(const uint8_t reg, const uint32_t size, const uint8_t* src) {
    uint8_t* buff = (uint8_t*)calloc(size + 1, 1);
    if (buff == NULL) {
        return; // Error handling
    }
    buff[0] = reg;
    for (uint32_t i = 0; i < size; i++) {
        buff[i + 1] = src[i];
    }
    i2c_write_blocking(i2c0, BMP280_I2C_ADDRESS, buff, size + 1, false);
    free(buff);
}

static void bmp280_read_reg(const uint8_t reg, const uint32_t size, uint8_t* dst) {
    i2c_write_blocking(i2c0, BMP280_I2C_ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c0, BMP280_I2C_ADDRESS, dst, size, false);
}

int bmp280_init() {
    sleep_ms(20);

    uint8_t chip_ID;
    bmp280_read_reg(BMP280_CHIP_ID_REG, 1, &chip_ID);
    if (chip_ID != BMP280_CHIP_ID) {
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
}

void bmp280_read_pressure(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_PRESSURE_REG_LOW, 3, data);

    int32_t adc_p = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int32_t var1, var2, p;
    var1 = (((device->temperature / 2.0) - 64000.0) * 5120.0);
    var1 += ((device->coefficients[17] << 8 | device->coefficients[16]) * var1 * var1) / 32768.0;
    var1 += ((device->coefficients[15] << 8 | device->coefficients[14]) * var1) * 2.0;
    var1 = (var1 / 4.0) + ((device->coefficients[13] << 8 | device->coefficients[12]) * 65536.0);
    var2 = ((device->coefficients[11] << 8 | device->coefficients[10]) * var1 * var1) / 524288.0;
    var1 = ((device->coefficients[9] << 8 | device->coefficients[8]) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * (device->coefficients[7] << 8 | device->coefficients[6]);
    p = 1048576 - adc_p;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((device->coefficients[23] << 8 | device->coefficients[22]) * p * p) / 2147483648.0;
    var2 = p * (device->coefficients[21] << 8 | device->coefficients[20]) / 32768.0;
    device->pressure = p + (var1 + var2 + ((device->coefficients[19] << 8 | device->coefficients[18]))) / 16.0;
}

void bmp280_read_temperature(bmp280* device) {
    uint8_t data[3];
    bmp280_read_reg(BMP280_TEMPERATURE_REG_LOW, 3, data);

    int32_t adc_t = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

    int32_t var1, var2;
    var1 = ((adc_t / 16384.0) - (device->coefficients[0] / 1024.0)) * device->coefficients[1];
    var2 = ((adc_t / 131072.0) - (device->coefficients[0] / 8192.0)) * (adc_t / 131072.0) * device->coefficients[2];
    device->temperature = (var1 + var2) / 5120.0;
}
