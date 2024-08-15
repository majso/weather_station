#include "nrf24l01.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <string.h>
#include <stdlib.h> // For malloc and free
#include <unistd.h> // For sleep (use if available for delays)

#define NRF24L01_CMD_R_REGISTER  0x00
#define NRF24L01_CMD_W_REGISTER  0x20
#define NRF24L01_CMD_FLUSH_TX    0xE1
#define NRF24L01_CMD_FLUSH_RX    0xE2
#define NRF24L01_CMD_REUSE_TX_PL 0xE3
#define NRF24L01_CMD_NOP         0xFF

// Define register addresses
#define NRF24L01_REG_CONFIG      0x00
#define NRF24L01_REG_EN_AA       0x01
#define NRF24L01_REG_EN_RXADDR   0x02
#define NRF24L01_REG_SETUP_AW    0x03
#define NRF24L01_REG_SETUP_RETR  0x04
#define NRF24L01_REG_RF_CH       0x05
#define NRF24L01_REG_RF_SETUP    0x06
#define NRF24L01_REG_STATUS      0x07
#define NRF24L01_REG_OBSERVE_TX  0x08
#define NRF24L01_REG_RPD         0x09
#define NRF24L01_REG_RX_ADDR_P0  0x0A
#define NRF24L01_REG_RX_ADDR_P1  0x0B
#define NRF24L01_REG_RX_ADDR_P2  0x0C
#define NRF24L01_REG_RX_ADDR_P3  0x0D
#define NRF24L01_REG_RX_ADDR_P4  0x0E
#define NRF24L01_REG_RX_ADDR_P5  0x0F
#define NRF24L01_REG_TX_ADDR     0x10
#define NRF24L01_REG_RX_PW_P0    0x11
#define NRF24L01_REG_RX_PW_P1    0x12
#define NRF24L01_REG_RX_PW_P2    0x13
#define NRF24L01_REG_RX_PW_P3    0x14
#define NRF24L01_REG_RX_PW_P4    0x15
#define NRF24L01_REG_RX_PW_P5    0x16
#define NRF24L01_REG_FIFO_STATUS 0x17

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
    gpio_set_function(NRF24L01_CS_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_CE_PIN, GPIO_FUNC_SIO);
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
