#include "nrf24l01.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h" // For sleep_ms
#include <string.h>

// Other functions...

void nrf24l01_init(nrf24l01_device *device) {
    // Initialize SPI
    spi_init(spi0, NRF24L01_SPI_SPEED);
    gpio_set_function(NRF24L01_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MISO_PIN, GPIO_FUNC_SPI);

    // Set GPIO directions
    gpio_set_dir(NRF24L01_CS_PIN, GPIO_OUT);
    gpio_set_dir(NRF24L01_CE_PIN, GPIO_OUT);
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    gpio_put(NRF24L01_CE_PIN, 0); // Set CE low

    // Initialize NRF24L01 settings
    nrf24l01_write_register(NRF24L01_REG_CONFIG, 0x0B); // Power up, CRC 16-bit, RX/TX
    nrf24l01_write_register(NRF24L01_REG_EN_AA, 0x00); // Disable auto-acknowledgment
    nrf24l01_write_register(NRF24L01_REG_EN_RXADDR, 0x01); // Enable RX pipe 0
    nrf24l01_write_register(NRF24L01_REG_SETUP_AW, 0x03); // 5-byte address width
    nrf24l01_write_register(NRF24L01_REG_SETUP_RETR, 0x03); // 1500us, 5 retransmits
    nrf24l01_write_register(NRF24L01_REG_RF_CH, 76); // Channel 76
    nrf24l01_write_register(NRF24L01_REG_RF_SETUP, 0x0F); // 1Mbps, 0dBm
    nrf24l01_write_register(NRF24L01_REG_STATUS, 0x70); // Clear interrupts
}

void nrf24l01_set_rx_address(nrf24l01_device *device, const uint8_t *address) {
    nrf24l01_write_registers(NRF24L01_REG_RX_ADDR_P0, address, 5);
}

void nrf24l01_set_tx_address(nrf24l01_device *device, const uint8_t *address) {
    nrf24l01_write_registers(NRF24L01_REG_TX_ADDR, address, 5);
}

bool nrf24l01_send(nrf24l01_device *device, const uint8_t *data, uint8_t length) {
    gpio_put(NRF24L01_CS_PIN, 0); // Select the NRF24L01
    spi_write_blocking(spi0, (const uint8_t[]){NRF24L01_CMD_W_REGISTER | 0xA0}, 1); // Write to TX payload
    spi_write_blocking(spi0, data, length);
    gpio_put(NRF24L01_CS_PIN, 1); // Deselect the NRF24L01

    // Wait for transmission to complete
    sleep_ms(1); // Adjust delay as necessary

    // Check if data has been transmitted
    uint8_t status = nrf24l01_read_register(NRF24L01_REG_STATUS);
    return (status & 0x20); // Check if TX_DS (data sent) bit is set
}

bool nrf24l01_receive(nrf24l01_device *device, uint8_t *data, uint8_t *length) {
    gpio_put(NRF24L01_CS_PIN, 0); // Select the NRF24L01
    spi_write_blocking(spi0, (const uint8_t[]){NRF24L01_CMD_R_REGISTER | 0x61}, 1); // Read RX payload
    spi_read_blocking(spi0, 0x00, data, 32); // Read up to 32 bytes
    gpio_put(NRF24L01_CS_PIN, 1); // Deselect the NRF24L01

    // Check if data has been received
    uint8_t status = nrf24l01_read_register(NRF24L01_REG_STATUS);
    return (status & 0x40); // Check if RX_DR (data ready) bit is set
}

void nrf24l01_power_up_rx(nrf24l01_device *device) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x01); // Set PWR_UP bit and RX mode
    gpio_put(NRF24L01_CE_PIN, 1); // Enable the receiver
}

void nrf24l01_power_up_tx(nrf24l01_device *device) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x01); // Set PWR_UP bit and TX mode
    gpio_put(NRF24L01_CE_PIN, 1); // Enable the transmitter
}

void nrf24l01_power_down(void) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config & ~0x02); // Clear PWR_UP bit to enter power-down mode
    gpio_put(NRF24L01_CE_PIN, 0); // Disable the module
}
