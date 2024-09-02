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
    cc1101_write_reg(CC1101_IOCFG0, 0x06);  // Configure GDO0 pin as FIFO is not empty
    cc1101_write_reg(CC1101_CHANNR, 0x00);  // Set channel
    cc1101_write_reg(CC1101_FREQ2, 0x21);   // Set frequency to a specific value
    cc1101_write_reg(CC1101_FREQ1, 0x65);
    cc1101_write_reg(CC1101_FREQ0, 0x6A);
    //MDMCFG2:
    //0 DC blocking
    //001 GFSK
    //0 Menchester
    //010 16/16 sync word has to match
    cc1101_write_reg(CC1101_MDMCFG2, 0x12); // Set modulation format (GFSK)
    // Set packet control:
    // Address filtering is handled separately from sync word detection. After detecting a 
    // sync word and pulling GDO0 high, the CC1101 will still need to check if the packet’s 
    // address matches the configured address (if address filtering is enabled).
    // If the address doesn’t match, the packet will be discarded, but this won’t affect 
    // the initial sync word detection signal from GDO0.
    cc1101_write_reg(CC1101_PKTCTRL1, 0x0F); // Enable address filtering
    cc1101_write_reg(CC1101_PKTCTRL0, 0x45); // Enable CRC and variable length mode
    // Sync word configuration
    // The SYNC_DETECT function is designed to trigger an interrupt when the sync word 
    // has been detected in a packet. This means that the GDO0 pin will go high when the 
    // CC1101 detects a sync word match, indicating the start of a packet.
    cc1101_write_reg(CC1101_SYNC1, 0xDE);  // SYNC1
    cc1101_write_reg(CC1101_SYNC0, 0xAD);  // SYNC0

    // Set device address
    cc1101_write_reg(CC1101_ADDR, 0x66); // Unique address for this device

    printf("Address Register: 0x%02X\n",  cc1101_read_reg(CC1101_ADDR));
    printf("PKTCTRL1 Register: 0x%02X\n", cc1101_read_reg(CC1101_PKTCTRL1));
    printf("SYNC1: 0x%02X\n", cc1101_read_reg(CC1101_SYNC1));
    printf("SYNC0: 0x%02X\n", cc1101_read_reg(CC1101_SYNC0));
    printf("PKTCTRL1: 0x%02X\n", cc1101_read_reg(CC1101_PKTCTRL1));
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

void print_binary(const uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            printf("%c", (buffer[i] & (1 << bit)) ? '1' : '0');
        }
    }
    printf("\n");
}

void radio_send_data(const SensorData *data) {
    uint8_t buffer[64];
    uint8_t length;

    // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);

    // Convert SensorData to byte array
    sensor_data_to_bytes(data, buffer, &length);

     // Check the initial state of GDO0
    printf("Initial GDO0 state: %d\n", gpio_get(CC1101_GDO0_PIN));

    // Write the data to the TX FIFO
    cc1101_send_data(buffer, length);

    // Optionally, wait until the TX FIFO is empty (GDO0 goes low again)
    while (gpio_get(CC1101_GDO0_PIN)) {
        printf("Waiting for GDO0 to go low (TX FIFO empty)...\n");
        sleep_ms(100); // Small delay to avoid flooding the console
    }
    printf("GDO0 is low (TX FIFO empty).\n");

    // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);
}

void radio_receive_data(SensorData *data) {
    uint8_t buffer[sizeof(SensorData)] = {0};

     // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);

     // Check the initial state of GDO0
    printf("Initial GDO0 state: %d\n", gpio_get(CC1101_GDO0_PIN));

    // Wait until a packet is received (GDO0 goes high)
    while (!gpio_get(CC1101_GDO0_PIN)) {
        printf("Waiting for GDO0 to go high (packet reception in progress)...\n");
        sleep_ms(1000); // Small delay to avoid flooding the console
    }
    printf("GDO0 is high (packet received).\n");

    // Read the data from the RX FIFO
    cc1101_receive_data(buffer, sizeof(SensorData));

    printf("Final GDO0 state: %d\n", gpio_get(CC1101_GDO0_PIN));

    // Print the entire packet in binary format
    printf("Received packet in binary: ");
    print_binary(buffer, sizeof(SensorData));

    uint8_t received_address = buffer[0];
    uint8_t expected_address = cc1101_read_reg(CC1101_ADDR);

    if (received_address != expected_address) {
        printf("Packet rejected: Address mismatch. Received address: 0x%02X\n", received_address);
        return;
    }

    // Convert the received buffer into a SensorData struct
    bytes_to_sensor_data(buffer, sizeof(SensorData), data);

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

     // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);
}