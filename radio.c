#include "nrf24l01.h" // Include the NRF24L01 library
#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <string.h>    // For memcpy
#include <stdio.h>

static nrf24l01_device radio_device;

// Initialize the radio module
void radio_init() {
    if (!nrf24l01_init(&radio_device)) {
        printf("Error initializing NRF24L01 radio\n");
        // Handle the error (e.g., reset the device, retry, etc.)
    } else {
        printf("NRF24L01 radio initialized successfully\n");
    }
}

// Send sensor data using the radio module
void radio_send_data(const SensorData *data) {
    if (data == NULL) {
        printf("No data to send\n");
        return;
    }
    // Convert SensorData to bytes (you may need to adjust this based on your data structure)
    uint8_t data_bytes[sizeof(SensorData)];
    memcpy(data_bytes, data, sizeof(SensorData));

    if (sizeof(SensorData) > 32) {
        printf("Error: Payload too big to send (max 32 bytes)\n");
        return;
    }   
    // Send the data
    if (!nrf24l01_send(&radio_device, data_bytes, sizeof(SensorData))) {
        printf("Failed to send data\n");
    }
}
