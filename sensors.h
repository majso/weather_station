#ifndef SENSORS_H
#define SENSORS_H

// Define I2C pins
#define I2C_SDA_PIN  14  //Black I2C Data pin (GPIO 14)
#define I2C_SCL_PIN  15  //White I2C Clock pin (GPIO 15)

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