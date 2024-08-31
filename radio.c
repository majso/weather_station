#include "cc1101.h"
#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Initialize the radio module
void radio_init() {
    // Initialize the CC1101 module
    cc1101_init();
    
    // Configure CC1101 registers as needed
    cc1101_write_reg(CC1101_IOCFG0, 0x06);  // Configure GDO0 pin as packet received signal
    cc1101_write_reg(CC1101_FREQ2, 0x21);   // Set frequency to a specific value
    cc1101_write_reg(CC1101_FREQ1, 0x65);
    cc1101_write_reg(CC1101_FREQ0, 0x6A);
    cc1101_write_reg(CC1101_MDMCFG4, 0x8C); // Set data rate and bandwidth
    cc1101_write_reg(CC1101_MDMCFG3, 0x22);
    cc1101_write_reg(CC1101_MDMCFG2, 0x02); // Set modulation format (GFSK)
    cc1101_write_reg(CC1101_PKTCTRL1, 0x04);// Enable automatic packet handling
    cc1101_write_reg(CC1101_PKTCTRL0, 0x05);// Enable CRC and variable length mode
}

// Helper function to convert SensorData to byte array
static void sensor_data_to_bytes(const SensorData *data, uint8_t *buffer, uint8_t *length) {
    // Use memcpy to pack floats into the byte array
    uint8_t index = 0;
    
    memcpy(&buffer[index], &data->temperature, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->pressure, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->exterior_temperature, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->exterior_humidity, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->battery_voltage, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->battery_current, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->battery_power, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->solar_voltage, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->solar_current, sizeof(float)); index += sizeof(float);
    memcpy(&buffer[index], &data->solar_power, sizeof(float)); index += sizeof(float);
    
    *length = index; // Total length of the data packet
}

// Helper function to convert byte array to SensorData
static void bytes_to_sensor_data(const uint8_t *buffer, uint8_t length, SensorData *data) {
    if (length >= sizeof(SensorData)) {
        uint8_t index = 0;
        
        memcpy(&data->temperature, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->pressure, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->exterior_temperature, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->exterior_humidity, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->battery_voltage, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->battery_current, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->battery_power, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->solar_voltage, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->solar_current, &buffer[index], sizeof(float)); index += sizeof(float);
        memcpy(&data->solar_power, &buffer[index], sizeof(float)); index += sizeof(float);
    }
}

// Send sensor data using the radio module
void radio_send_data(const SensorData *data) {
    uint8_t buffer[64];
    uint8_t length;
    
    // Convert SensorData to byte array
    sensor_data_to_bytes(data, buffer, &length);
    
    // Send the data using CC1101
    cc1101_send_data(buffer, length);
}

// Receive sensor data using the radio module
void radio_receive_data(SensorData *data) {
    uint8_t buffer[64];
    uint8_t length = sizeof(buffer);
    
    // Receive data from CC1101
    cc1101_receive_data(buffer, length);
    
    // Convert received bytes to SensorData
    bytes_to_sensor_data(buffer, length, data);
}
