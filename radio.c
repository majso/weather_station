#include "nrf24l01.h" // Include the NRF24L01 library
#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <string.h>    // For memcpy
#include <stdbool.h> // For bool type
#include "pico/stdlib.h"
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

    uint8_t data_bytes[sizeof(SensorData)];
    memcpy(data_bytes, data, sizeof(SensorData));

    size_t total_length = sizeof(data_bytes);
    size_t offset = 0;

    while (offset < total_length) {
        size_t chunk_size = (total_length - offset > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : (total_length - offset);
        
        // Send the data chunk
        if (!nrf24l01_send(&radio_device, data_bytes + offset, chunk_size)) {
            printf("Failed to send data chunk of size %zu bytes\n", chunk_size);
        } else {
            printf("Successfully sent data chunk of size %zu bytes\n", chunk_size);
        }

        offset += chunk_size;

        // Wait for the transmission to complete
        sleep_ms(10); // Adjust this delay based on your requirements
    }
}

// Receive sensor data using the radio module
bool radio_receive_data(SensorData *data) {
    uint8_t data_bytes[sizeof(SensorData)];
    uint8_t length = sizeof(data_bytes);
    size_t offset = 0;

    while (offset < sizeof(data_bytes)) {
        if (!nrf24l01_receive(&radio_device, data_bytes + offset, &length)) {
            printf("Failed to receive data chunk\n");
            return false;
        }
        offset += length;
        sleep_ms(10); // Short delay to avoid collision and buffer overflow
    }

    memcpy(data, data_bytes, sizeof(SensorData));
    return true;
}

// Switch between TX and RX modes
void radio_switch_mode(bool is_transmitting) {
    if (is_transmitting) {
        nrf24l01_power_up_tx(&radio_device);
        gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start TX mode
    } else {
        nrf24l01_power_up_rx(&radio_device);
        gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start RX mode
    }
}