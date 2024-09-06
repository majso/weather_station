#ifndef SENSORS_H
#define SENSORS_H

// Define I2C pins
#define I2C_SDA_PIN  14  //Black I2C Data pin (GPIO 14)
#define I2C_SCL_PIN  15  //White I2C Clock pin (GPIO 15)

#define I2C_FREQ_HZ  100000
#define I2C_BUS_INSTANCE i2c1

#define BMP280_I2C_ADDRESS 0x76
#define SHT40_I2C_ADDRESS 0x44
#define INA219_I2C_ADDRESS 0x40

#define SEA_LEVEL_PRESSURE_HPA 1013.25 // Standard sea level pressure in hPa
#define SEA_LEVEL_PRESSURE_PA (SEA_LEVEL_PRESSURE_HPA * 100.0f) // Convert hPa to Pa
#define TEMPERATURE_LAPSE_RATE 0.0065 // Temperature lapse rate in K/m (average)
#define GRAVITY_ACCELERATION 9.80665 // Acceleration due to gravity in m/s^2
#define MOLAR_MASS_AIR 0.0289644 // Molar mass of air in kg/mol
#define UNIVERSAL_GAS_CONSTANT 8.31447 // Universal gas constant in J/(mol·K)
#define SEA_LEVEL_TEMP_K 288.15f // Standard sea-level temperature in Kelvin

typedef struct {
    float temperature;
    float pressure;
    float exterior_temperature;
    float exterior_humidity;
    float battery_voltage;
    float battery_current;
    float battery_power;
    float solar_voltage;
    float solar_current;
    float solar_power;
} SensorData;

void sensors_init(void);
SensorData sensors_read_all(void);

#endif