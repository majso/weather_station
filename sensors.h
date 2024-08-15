#ifndef SENSORS_H
#define SENSORS_H

typedef struct {
    float temperature;
    float pressure;
    float humidity;
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