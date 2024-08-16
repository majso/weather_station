#include "nrf24l01.h" // Include the NRF24L01 library
#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <stdio.h>

static nrf24l01_device radio_device;

// Initialize the radio module
void radio_init() {
    // Initialize SPI (using pins smaller than 13)
    spi_init(spi0, 8 * 1000 * 1000); // 8 MHz
    gpio_set_function(6, GPIO_FUNC_SPI); // SCK (GPIO 6)
    gpio_set_function(7, GPIO_FUNC_SPI); // MOSI (GPIO 7)
    gpio_set_function(4, GPIO_FUNC_SPI); // MISO (GPIO 4)

    // Set CSN and CE as output
    gpio_init(5); // CSN (GPIO 5)
    gpio_set_dir(5, GPIO_OUT);
    gpio_put(5, 1); // Set CSN high

    gpio_init(3); // CE (GPIO 3)
    gpio_set_dir(3, GPIO_OUT);
    gpio_put(3, 0); // Set CE low (initially)

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
