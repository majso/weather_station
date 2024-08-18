#ifndef INA219_H
#define INA219_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// Define INA219 register addresses
#define INA219_REG_CONFIG 0x00
#define INA219_REG_CALIBRATION 0x05
#define INA219_REG_BUSVOLTAGE 0x02
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_CURRENT 0x04
#define INA219_REG_POWER 0x03

typedef struct {
    i2c_inst_t *i2c_instance;
    uint8_t i2c_addr;
    float current_LSB;
} INA219;

// Function prototypes
bool ina219_init(INA219 *ina219, i2c_inst_t *i2c_instance, uint8_t i2c_addr);
uint16_t ina219_read_register(INA219 *ina219, uint8_t reg);
void ina219_write_register(INA219 *ina219, uint8_t reg, uint16_t value);
void ina219_calibrate(INA219 *ina219, float shunt_resistor_value, float max_expected_amps);
float ina219_read_voltage(INA219 *ina219);
float ina219_read_shunt_voltage(INA219 *ina219);
float ina219_read_current(INA219 *ina219);
float ina219_read_power(INA219 *ina219);

#endif