#ifndef SHT30_H
#define SHT30_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// SHT30 I2C Address
#define SHT30_I2C_ADDR 0x44

// SHT30 Commands
#define SHT30_MEASURE_HIGHREP_STRETCH 0x2400
#define SHT30_MEASURE_MEDREP_STRETCH 0x240B
#define SHT30_MEASURE_LOWREP_STRETCH 0x2416
#define SHT30_SOFT_RESET 0x30A2

// Function prototypes
void sht30_init(i2c_inst_t *i2c_instance, uint8_t i2c_addr);
void sht30_soft_reset();
bool sht30_read_data(float *temperature, float *humidity);

#endif
