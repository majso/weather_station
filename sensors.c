#include "sensors.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "INA219.h"
#include "sht30.h" // Include the header file for the SHT30 sensor
#include "bmp280.h" // Include the header file for the BMP280 sensor
#include <stdio.h>

INA219 ina219_battery;
INA219 ina219_solar;
bmp280 bmp;

// Initialize sensors
void sensors_init() {
    // Initialize I2C with GPIO 14 and 15
    i2c_init(i2c0, 100 * 1000); // 100kHz
    printf("Starting I2C\n");
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Initialize INA219 sensors
    
    ina219_init(&ina219_solar, i2c0, 0x40);
    printf("INA219 Solar sensor initialized\n");

    ina219_init(&ina219_battery, i2c0, 0x41);
    printf("INA219 Battery sensor initialized\n");

    // Initialize SHT30 sensor
    //sht30_init(i2c0, 0x42); // Replace with your SHT30 address if different
    // Initialize BMP280 sensor
    //bmp280_init();
   
    printf("All sensors initialized\n");
}
// Read current from INA219 sensor// Read current from INA219 sensor
float read_current_from_ina219(INA219 *ina219, uint8_t address) {
    ina219_init(ina219, i2c0, address);
    printf("Reading Current from INA219\n");
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
// Read temperature from BMP280 sensor
float read_temperature_from_bmp280() {
    bmp280_read_temperature(&bmp);
    return bmp.temperature;
}

// Read pressure from BMP280 sensor
float read_pressure_from_bmp280() {
    bmp280_read_pressure(&bmp);
    return bmp.pressure;
}


// Read all sensor data
// Read all sensor data
SensorData sensors_read_all() {
    SensorData data = {0};

    // Read temperature and pressure from BMP280
    data.temperature = read_temperature_from_bmp280();
    data.pressure = read_pressure_from_bmp280();

    // Read temperature and humidity from SHT30 sensor
    data.exterior_temperature = read_temperature_from_sht30();
    data.exterior_humidity = read_humidity_from_sht30();

    // Read battery data from INA219 sensor
    data.battery_voltage = read_voltage_from_ina219(&ina219_battery, 0x41);
    data.battery_current = read_current_from_ina219(&ina219_battery, 0x41);
    data.battery_power = read_power_from_ina219(&ina219_battery, 0x41);

    // Read solar data from INA219 sensor
    data.solar_voltage = read_voltage_from_ina219(&ina219_solar, 0x40);
    data.solar_current = read_current_from_ina219(&ina219_solar, 0x40);
    printf("Solar current: %f\n", data.solar_current);
    data.solar_power = read_power_from_ina219(&ina219_solar, 0x40);

    return data;
}
