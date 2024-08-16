#include "sensors.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "INA219.h"
#include "sht30.h" // Include the header file for the SHT30 sensor
#include "bmp280.h" // Include the header file for the BMP280 sensor
#include <stdio.h>

// Initialize sensors
void sensors_init() {
    // Initialize I2C (assuming default I2C0 pins)
    i2c_init(i2c0, 100 * 1000); // 100kHz
    printf("Starting i2c\n");
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    // Initialize INA219 sensors (assuming default addresses)
    INA219 ina219_battery;
    INA219 ina219_solar;
    ina219_init(&ina219_battery, i2c0, 0x41);
    printf("Starting ina219_battery\n");

    ina219_init(&ina219_solar, i2c0, 0x40);
    printf("Starting ina219_solar\n");

    // Initialize SHT30 sensor
    sht30_init(i2c0, 0x44); // Replace with your SHT30 address if different
    printf("Starting sht30\n");
    sht30_soft_reset();
}
// Read current from INA219 sensor// Read current from INA219 sensor
float read_current_from_ina219(INA219 *ina219, uint8_t address) {
    ina219_init(ina219, i2c0, address);
    return ina219_read_current(ina219);
}

// Read power from INA219 sensor
float read_power_from_ina219(INA219 *ina219, uint8_t address) {
    ina219_init(ina219, i2c0, address);
    return ina219_read_power(ina219);
}

// Read voltage from INA219 sensor
float read_voltage_from_ina219(INA219 *ina219, uint8_t address) {
    ina219_init(ina219, i2c0, address);
    return ina219_read_voltage(ina219);
}

// Read temperature from SHT30 sensor
float read_temperature_from_sht30() {
    float temperature, humidity;
    sht30_read_data(&temperature, &humidity);
        return temperature;
}

// Read humidity from SHT30 sensor
float read_humidity_from_sht30() {
    float temperature, humidity;
    sht30_read_data(&temperature, &humidity);
        return humidity;
}

float read_temperature_from_bmp280(bmp280 *device) {
    bmp280_read_temperature(device);
        return device->temperature;
}

float read_pressure_from_bmp280(bmp280 *device) {
    bmp280_read_pressure(device);
        return device->pressure;
}

// Read all sensor data
SensorData sensors_read_all() {
    SensorData data;

    bmp280_init();
    bmp280 bmp;
    bmp280_calibrate(&bmp);

    // Read temperature and pressure
    data.temperature = read_temperature_from_bmp280(&bmp);
    data.pressure = read_pressure_from_bmp280(&bmp);

    // Read temperature, pressure, and humidity
    data.temperature = read_temperature_from_sht30();
    data.humidity = read_humidity_from_sht30();

    // Read battery data
    INA219 ina219_battery;
    data.battery_voltage = read_voltage_from_ina219(&ina219_battery, 0x41);
    data.battery_current = read_current_from_ina219(&ina219_battery, 0x41);
    data.battery_power = read_power_from_ina219(&ina219_battery, 0x41);

    // Read solar data
    INA219 ina219_solar;
    data.solar_voltage = read_voltage_from_ina219(&ina219_solar, 0x40);
    data.solar_current = read_current_from_ina219(&ina219_solar, 0x40);
    data.solar_power = read_power_from_ina219(&ina219_solar, 0x40);

    return data;
}
