#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

#define BMP280_I2C_ADDRESS 0x43

#define BMP280_CHIP_ID_REG 0xD0
#define BMP280_CHIP_ID 0x58

#define BMP280_RESET_REG 0xE0
#define BMP280_RESET_VAL 0xB6

#define BMP280_POWER_CTL_REG 0xF4
#define BMP280_OVERSCAN_X2 1
#define BMP280_OVERSCAN_X16 2
#define BMP280_MODE_NORMAL 3

#define BMP280_PRESSURE_REG_LOW 0xF7
#define BMP280_TEMPERATURE_REG_LOW 0xFA

typedef struct {
    double temperature;
    double pressure;
    uint8_t coefficients[24];
} bmp280;

int bmp280_init();
void bmp280_calibrate(bmp280* device);
void bmp280_read_pressure(bmp280* device);
void bmp280_read_temperature(bmp280* device);

#endif // BMP280_H
