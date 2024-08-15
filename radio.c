#include "nrf24l01.h" // Include the NRF24L01 library
#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <stdio.h>

#define RADIO_CS_PIN 10  // Define your Chip Select (CS) pin
#define RADIO_CE_PIN 9   // Define your Chip Enable (CE) pin

static nrf24l01_device radio_device;

// Initialize the radio module
void radio_init() {
    // Initialize SPI (assuming default SPI0 pins)
    spi_init(spi0, 8 * 1000 * 1000); // 8 MHz
    gpio_set_function(16, GPIO_FUNC_SPI); // SCK
    gpio_set_function(17, GPIO_FUNC_SPI); // TX
    gpio_set_function(18, GPIO_FUNC_SPI); // RX

    // Initialize GPIO pins for nRF24L01+
    gpio_set_dir(RADIO_CS_PIN, GPIO_OUT);
    gpio_set_dir(RADIO_CE_PIN, GPIO_OUT);
    gpio_put(RADIO_CS_PIN, 1); // Set CS high
    gpio_put(RADIO_CE_PIN, 0); // Set CE low

    // Initialize nRF24L01+
    radio_device.tx_buffer = NULL;  // You can allocate memory if needed
    radio_device.rx_buffer = NULL;  // You can allocate memory if needed
    nrf24l01_init(&radio_device);
    nrf24l01_set_rx_address(&radio_device, (const uint8_t[]){0xE7, 0xE7, 0xE7, 0xE7, 0xE7}); // Example address
    nrf24l01_set_tx_address(&radio_device, (const uint8_t[]){0xE7, 0xE7, 0xE7, 0xE7, 0xE7}); // Example address
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

    // Send the data
    if (!nrf24l01_send(&radio_device, data_bytes, sizeof(SensorData))) {
        printf("Failed to send data\n");
    }
}
