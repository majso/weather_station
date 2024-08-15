#include "radio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <stdio.h>

void radio_init() {
    // Initialize SPI (assuming default SPI0 pins)
    spi_init(spi0, 8 * 1000 * 1000); // 8 MHz
    gpio_set_function(16, GPIO_FUNC_SPI); // SCK
    gpio_set_function(17, GPIO_FUNC_SPI); // TX
    gpio_set_function(18, GPIO_FUNC_SPI); // RX

    // Initialize nRF24L01+ (specific to your module)
    // Example: nrf24_init(), nrf24_set_channel(), etc.
}

void radio_send_data(const SensorData *data) {
    // Convert data to bytes and send via nRF24L01+
    // Example: nrf24_send(data, sizeof(SensorData));
}