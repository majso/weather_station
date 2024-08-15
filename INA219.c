#include "ina219.h"

void ina219_init(INA219 *ina219, i2c_inst_t *i2c_instance, uint8_t i2c_addr) {
    ina219->i2c_instance = i2c_instance;
    ina219->i2c_addr = i2c_addr;
    ina219->current_LSB = 0.0;
}

void ina219_start(INA219 *ina219, uint32_t sda_pin, uint32_t scl_pin, uint32_t speed_khz) {
    i2c_init(ina219->i2c_instance, speed_khz * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

uint16_t ina219_read_register(INA219 *ina219, uint8_t reg) {
    uint8_t buf[2];
    i2c_write_blocking(ina219->i2c_instance, ina219->i2c_addr, &reg, 1, true);
    i2c_read_blocking(ina219->i2c_instance, ina219->i2c_addr, buf, 2, false);
    return (buf[0] << 8) | buf[1];
}

void ina219_write_register(INA219 *ina219, uint8_t reg, uint16_t value) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = value & 0xFF;
    i2c_write_blocking(ina219->i2c_instance, ina219->i2c_addr, buf, 3, false);
}

void ina219_calibrate(INA219 *ina219, float shunt_resistor_value, float max_expected_amps) {
    ina219->current_LSB = max_expected_amps / 32768.0;
    uint16_t cal_reg_value = (uint16_t)(0.04096 / (ina219->current_LSB * shunt_resistor_value));
    ina219_write_register(ina219, INA219_REG_CALIBRATION, cal_reg_value);
}

float ina219_read_voltage(INA219 *ina219) {
    uint16_t value = ina219_read_register(ina219, INA219_REG_BUSVOLTAGE);
    return (value >> 3) * 0.004;
}

float ina219_read_shunt_voltage(INA219 *ina219) {
    int16_t value = (int16_t)ina219_read_register(ina219, INA219_REG_SHUNTVOLTAGE);
    return value * 0.01;
}

float ina219_read_current(INA219 *ina219) {
    int16_t value = (int16_t)ina219_read_register(ina219, INA219_REG_CURRENT);
    return value * ina219->current_LSB;
}

float ina219_read_power(INA219 *ina219) {
    uint16_t value = ina219_read_register(ina219, INA219_REG_POWER);
    return value * 0.02;
}