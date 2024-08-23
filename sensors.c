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

// Function to scan the I2C bus for devices
void i2c_scan(i2c_inst_t *i2c_instance) {
    printf("Scanning I2C bus...\n");
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        uint8_t data = 0;
        int ret = i2c_write_blocking(i2c_instance, addr, &data, 1, true);
        if (ret == 1) {
            printf("Device found at address: 0x%02X\n", addr);
        }
    }
    printf("I2C scan complete.\n");
}
// Initialize sensors
void sensors_init() {
    // Initialize I2C
    i2c_init(I2C_BUS_INSTANCE, I2C_FREQ_HZ); 
    printf("Starting I2C\n");

    // Set GPIO pins for I2C functionality
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // Enable internal pull-up resistors
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Scan the I2C bus
    i2c_scan(I2C_BUS_INSTANCE);     

    // Initialize INA219 sensors
    
    ina219_init(&ina219_solar, I2C_BUS_INSTANCE, INA219_I2C_ADDRESS);
    printf("INA219 Solar sensor initialized\n");

    //ina219_init(&ina219_battery, i2c0, 0x41);
    //printf("INA219 Battery sensor initialized\n");

    // Initialize SHT30 sensor
    sht30_init(I2C_BUS_INSTANCE, SHT30_I2C_ADDRESS); // Replace with your SHT30 address if different
    printf("SHT30 sensor initialized\n");
    // Initialize BMP280 sensor
    bmp280_init(I2C_BUS_INSTANCE, BMP280_I2C_ADDRESS);
    bmp280_calibrate(&bmp);
    printf("BMP280 sensor initialized\n");
   
    printf("All sensors initialized\n");
}

// Read current from INA219 sensor
float read_current_from_ina219(INA219 *ina219) {
    printf("Reading Current from INA219\n");
    return ina219_read_current(ina219);
}

// Read power from INA219 sensor
float read_power_from_ina219(INA219 *ina219) {
    return ina219_read_power(ina219);
}

// Read voltage from INA219 sensor
float read_voltage_from_ina219(INA219 *ina219) {
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

    /*
    // Read battery data from INA219 sensor
    data.battery_voltage = read_voltage_from_ina219(&ina219_battery);
    data.battery_current = read_current_from_ina219(&ina219_battery);
    data.battery_power = read_power_from_ina219(&ina219_battery);
    */
    // Read solar data from INA219 sensor
    printf("Reading solar data\n");
    data.solar_voltage = read_voltage_from_ina219(&ina219_solar);
    printf("Solar voltage: %f\n", data.solar_voltage);
    data.solar_current = read_current_from_ina219(&ina219_solar);
    printf("Solar current: %f\n", data.solar_current);
    data.solar_power = read_power_from_ina219(&ina219_solar);
    printf("Solar power: %f\n", data.solar_power);

  // Read temperature and humidity from SHT30 sensor
    printf("Reading temperature and humidity from SHT30\n");
    data.exterior_temperature = read_temperature_from_sht30();
    printf("Temperature: %f\n", data.exterior_temperature);
    data.exterior_humidity = read_humidity_from_sht30();
    printf("Humidity: %f\n", data.exterior_humidity);

    // Read temperature and pressure from BMP280
    printf("Reading temperature and pressure from BMP280\n");
    data.temperature = read_temperature_from_bmp280();
    printf("Temperature: %f\n", data.temperature);
    data.pressure = read_pressure_from_bmp280();
    printf("Pressure: %f\n", data.pressure);

  

    return data;
}
