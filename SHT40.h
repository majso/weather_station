#ifndef SHT40_H
#define SHT40_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// SHT40 I2C Address
#define SHT40_I2C_ADDR 0x44

// SHT40 Commands
#define SHT40_MEASURE_HIGHREP_STRETCH 0xFD
#define SHT40_MEASURE_MEDREP_STRETCH 0xF6
#define SHT40_MEASURE_LOWREP_STRETCH 0xE0
#define SHT40_SOFT_RESET 0x94

// Function prototypes
void sht40_init(i2c_inst_t *i2c_instance, uint8_t i2c_addr);
bool sht40_read_data(float *temperature, float *humidity);

#endif
