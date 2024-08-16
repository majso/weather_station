#include "nrf24l01.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include <string.h>    // For memcpy
#include <stdlib.h>    // For malloc and free
#include <unistd.h>    // For sleep (use if available for delays)

// Helper function to write a single register
static void nrf24l01_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {NRF24L01_CMD_W_REGISTER | reg, value};
    spi_write_blocking(spi0, buffer, sizeof(buffer));
}

// Helper function to read a single register
static uint8_t nrf24l01_read_register(uint8_t reg) {
    uint8_t buffer[2] = {NRF24L01_CMD_R_REGISTER | reg, 0};
    spi_write_read_blocking(spi0, buffer, buffer, sizeof(buffer));
    return buffer[1];
}

// Helper function to write multiple bytes
static void nrf24l01_write_registers(uint8_t reg, const uint8_t *values, uint8_t length) {
    uint8_t *buffer = (uint8_t *)malloc(length + 1);
    if (buffer) {
        buffer[0] = NRF24L01_CMD_W_REGISTER | reg;
        memcpy(buffer + 1, values, length);
        spi_write_blocking(spi0, buffer, length + 1);
        free(buffer);
    }
}

// Helper function to read multiple bytes
static void nrf24l01_read_registers(uint8_t reg, uint8_t *values, uint8_t length) {
    uint8_t *buffer = (uint8_t *)malloc(length + 1);
    if (buffer) {
        buffer[0] = NRF24L01_CMD_R_REGISTER | reg;
        spi_write_read_blocking(spi0, buffer, buffer, length + 1);
        memcpy(values, buffer + 1, length);
        free(buffer);
    }
}

void nrf24l01_init(nrf24l01_device *device) {
    // SPI initialization
    spi_init(spi0, NRF24L01_SPI_SPEED);
    gpio_set_function(NRF24L01_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_dir(NRF24L01_CS_PIN, GPIO_OUT);
    gpio_set_dir(NRF24L01_CE_PIN, GPIO_OUT);
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high

    // Initialize NRF24L01
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
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x01); // Power up and enable RX
    gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start RX mode
}

void nrf24l01_power_up_tx(nrf24l01_device *device) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x02); // Power up and enable TX
    gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start TX mode
}

void nrf24l01_power_down(void) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config & ~0x02); // Power down
    gpio_put(NRF24L01_CE_PIN, 0); // Set CE low
}
