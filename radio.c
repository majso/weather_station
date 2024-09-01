#include "cc1101.h"
#include "radio.h"
#include "hardware/gpio.h"
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

void radio_send_data(const SensorData *data) {
    uint8_t buffer[64];
    uint8_t length;
    
    // Convert SensorData to byte array
    sensor_data_to_bytes(data, buffer, &length);
    
    // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);

    // Write the data to the TX FIFO
    cc1101_write_burst(CC1101_TXFIFO_BURST, buffer, length);

    // Set the CC1101 to TX mode to send the data
    cc1101_strobe(CC1101_STX);

    // Wait for transmission to complete (GDO0 goes high)
    while (!gpio_get(CC1101_GDO0_PIN));

    // Optionally, wait until the TX FIFO is empty (GDO0 goes low again)
    while (gpio_get(CC1101_GDO0_PIN));
}

void radio_receive_data(SensorData *data) {
    uint8_t buffer[sizeof(SensorData)] = {0};

    // Set the CC1101 to RX mode
    cc1101_strobe(CC1101_SRX);

    // Wait until a packet is received (GDO0 goes high)
    while (!gpio_get(CC1101_GDO0_PIN));

    // Read the data from the RX FIFO
    cc1101_read_burst(CC1101_RXFIFO_BURST, buffer, sizeof(SensorData));

    // Convert the received buffer into a SensorData struct
    memcpy(data, buffer, sizeof(SensorData));

    // Print the received data
    printf("Temperature: %.2f°C\n", data->temperature);
    printf("Pressure: %.2f hPa\n", data->pressure);
    printf("Exterior Temperature: %.2f°C\n", data->exterior_temperature);
    printf("Exterior Humidity: %.2f%%\n", data->exterior_humidity);
    printf("Battery Voltage: %.2fV\n", data->battery_voltage);
    printf("Battery Current: %.2fA\n", data->battery_current);
    printf("Battery Power: %.2fW\n", data->battery_power);
    printf("Solar Voltage: %.2fV\n", data->solar_voltage);
    printf("Solar Current: %.2fA\n", data->solar_current);
    printf("Solar Power: %.2fW\n", data->solar_power);
}
