#include "cc1101.h"
#include "radio.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Initialize the radio module
void radio_init(uint8_t f) {
    // Initialize the CC1101 module
    cc1101_init();
    
    cc1101_write_reg(CC1101_FSCTRL1,  0x08);
    cc1101_write_reg(CC1101_FSCTRL0,  0x00);
	
    switch(f)
    {
      case F_868:
      	cc1101_write_reg(CC1101_FREQ2,    F2_868);
      	cc1101_write_reg(CC1101_FREQ1,    F1_868);
      	cc1101_write_reg(CC1101_FREQ0,    F0_868);
        break;
      case F_915:
        cc1101_write_reg(CC1101_FREQ2,    F2_915);
        cc1101_write_reg(CC1101_FREQ1,    F1_915);
        cc1101_write_reg(CC1101_FREQ0,    F0_915);
        break;
	  case F_433:
        cc1101_write_reg(CC1101_FREQ2,    F2_433);
        cc1101_write_reg(CC1101_FREQ1,    F1_433);
        cc1101_write_reg(CC1101_FREQ0,    F0_433);
        break;
	  default: // F must be set
	  	break;
	}
	
    cc1101_write_reg(CC1101_MDMCFG4,  0x5B);
    cc1101_write_reg(CC1101_MDMCFG3,  0xF8);
    //MDMCFG2:
    //0 DC blocking
    //001 GFSK
    //0 Menchester
    //010 16/16 sync word has to match
    cc1101_write_reg(CC1101_MDMCFG2,  0x12); // Set modulation format (GFSK)
    cc1101_write_reg(CC1101_MDMCFG1,  0x40); // 8 byte preamble
    cc1101_write_reg(CC1101_MDMCFG0,  0xF8);
    cc1101_write_reg(CC1101_CHANNR,   0x00);
    cc1101_write_reg(CC1101_DEVIATN,  0x47);
    cc1101_write_reg(CC1101_FREND1,   0xB6);
    cc1101_write_reg(CC1101_FREND0,   0x10);
    cc1101_write_reg(CC1101_MCSM0 ,   0x18);
    cc1101_write_reg(CC1101_FOCCFG,   0x1D);
    cc1101_write_reg(CC1101_BSCFG,    0x1C);
    cc1101_write_reg(CC1101_AGCCTRL2, 0xC7);
	cc1101_write_reg(CC1101_AGCCTRL1, 0x00);
    cc1101_write_reg(CC1101_AGCCTRL0, 0xB2);
    cc1101_write_reg(CC1101_FSCAL3,   0xEA);
	cc1101_write_reg(CC1101_FSCAL2,   0x2A);
	cc1101_write_reg(CC1101_FSCAL1,   0x00);
    cc1101_write_reg(CC1101_FSCAL0,   0x11);
    cc1101_write_reg(CC1101_FSTEST,   0x59);
    cc1101_write_reg(CC1101_TEST2,    0x81);
    cc1101_write_reg(CC1101_TEST1,    0x35);
    cc1101_write_reg(CC1101_TEST0,    0x09);
    cc1101_write_reg(CC1101_IOCFG0,   0x06); 
    cc1101_write_reg(CC1101_PKTCTRL1, 0xFF); // Enable address filtering, auto Flush
    cc1101_write_reg(CC1101_PKTCTRL0, 0x45); // Enable CRC and variable length mode
    // Set packet control:
    // Address filtering is handled separately from sync word detection. After detecting a 
    // sync word and pulling GDO0 high, the CC1101 will still need to check if the packet’s 
    // address matches the configured address (if address filtering is enabled).
    // If the address doesn’t match, the packet will be discarded, but this won’t affect 
    // the initial sync word detection signal from GDO0.
    cc1101_write_reg(CC1101_PKTLEN, 0x3D); // 61 bytes
    cc1101_write_reg(CC1101_MCSM1,0x30); // CCA enabled TX->IDLE RX->IDLE
    // Sync word configuration
    // The SYNC_DETECT function is designed to trigger an interrupt when the sync word 
    // has been detected in a packet. This means that the GDO0 pin will go high when the 
    // CC1101 detects a sync word match, indicating the start of a packet.
    cc1101_write_reg(CC1101_SYNC1, 0xDE);  // SYNC1
    cc1101_write_reg(CC1101_SYNC0, 0xAD);  // SYNC0

    // Set device address
    cc1101_write_reg(CC1101_ADDR, 0x66); // Unique address for this device

}

// Helper function to convert SensorData to byte array
static void sensor_data_to_bytes(const SensorData *data, uint8_t *buffer, uint8_t *length) {
    // Use memcpy to pack floats into the byte array
    uint8_t byte_index = 0;
    
    memcpy(&buffer[byte_index], &data->temperature, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->pressure, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->exterior_temperature, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->exterior_humidity, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->battery_voltage, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->battery_current, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->battery_power, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->solar_voltage, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->solar_current, sizeof(float)); byte_index += sizeof(float);
    memcpy(&buffer[byte_index], &data->solar_power, sizeof(float)); byte_index += sizeof(float);
    
    *length = byte_index; // Total length of the data packet
}

// Helper function to convert byte array to SensorData
void bytes_to_sensor_data(const uint8_t *buffer, uint8_t *packet_length, uint8_t *address, SensorData *data) {
    if (buffer == NULL || data == NULL || packet_length == NULL || address == NULL) {
        printf("Invalid pointer passed to bytes_to_sensor_data.\n");
        return;
    }

    // Read length and address from buffer
    *packet_length = buffer[0];  // Length of the packet
    *address = buffer[1];        // Address

    // Set up byte_index to point to the start of the payload
    uint8_t byte_index = 2;

    // Convert the payload bytes to SensorData structure
    memcpy(&data->temperature, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->pressure, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->exterior_temperature, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->exterior_humidity, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->battery_voltage, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->battery_current, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->battery_power, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->solar_voltage, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->solar_current, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
    memcpy(&data->solar_power, &buffer[byte_index], sizeof(float)); byte_index += sizeof(float);
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
    uint8_t buffer[64] = {0};
    uint8_t length;
    uint8_t address = cc1101_read_reg(CC1101_ADDR);

    // Convert SensorData to byte array
    sensor_data_to_bytes(data, buffer, &length);

     // Check the initial state of GDO0
    printf("Initial GDO0 state: %d\n", gpio_get(CC1101_GDO0_PIN));

    // Write the data to the TX FIFO
    cc1101_send_data(buffer, length, address);
}

void radio_receive_data(SensorData *data) {
    uint8_t buffer[64] = {0};
    uint8_t length;
    uint8_t address = cc1101_read_reg(CC1101_ADDR);

     // Set the CC1101 to IDLE mode
    cc1101_strobe(CC1101_SIDLE);
    cc1101_strobe(CC1101_SRX);

     // Check the initial state of GDO0
    printf("Initial GDO0 state: %d\n", gpio_get(CC1101_GDO0_PIN));

    // Wait until a packet is received (GDO0 goes high)
    while (gpio_get(CC1101_GDO0_PIN)) {
        printf("Waiting for GDO0 to go high (packet reception in progress)...\n");
        sleep_ms(1000); // Small delay to avoid flooding the console
    }
    printf("GDO0 is high (packet received).\n");

    // Read the data from the RX FIFO
    cc1101_receive_data(buffer, &length);

    if (length == 0) {
        printf("No data processed.\n");
        return;
    }
    // Print the entire packet in binary format
    printf("Received packet in binary: ");
    print_binary(buffer, length);

    uint8_t packet_length;
    uint8_t packet_address;
    // Convert the received buffer into a SensorData struct
    bytes_to_sensor_data(buffer, &packet_length, &packet_address, data);
    // Print the length
    printf("Length: %d\n", packet_length);
    // Print the address in hexadecimal format
    printf("Address: 0x%02X\n", packet_address);
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