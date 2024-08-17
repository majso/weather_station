#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include <stdbool.h>

// SPI configuration
#define NRF24L01_SPI_SPEED    1000000 // 1 MHz, adjust if needed
#define NRF24L01_CS_PIN       5       // Chip Select (CS) pin (GPIO 5)
#define NRF24L01_CE_PIN       3       // Chip Enable (CE) pin (GPIO 3)

// SPI0 Pin Definitions
#define NRF24L01_SPI_SCK_PIN  6       // SPI Clock (SCK) pin (GPIO 6)
#define NRF24L01_SPI_MOSI_PIN 7       // SPI Master Out Slave In (MOSI) pin (GPIO 7)
#define NRF24L01_SPI_MISO_PIN 4       // SPI Master In Slave Out (MISO) pin (GPIO 4)

// NRF24L01 Commands
#define NRF24L01_CMD_R_REGISTER  0x00
#define NRF24L01_CMD_W_REGISTER  0x20
#define NRF24L01_CMD_FLUSH_TX    0xE1
#define NRF24L01_CMD_FLUSH_RX    0xE2
#define NRF24L01_CMD_REUSE_TX_PL 0xE3
#define NRF24L01_CMD_NOP         0xFF

// NRF24L01 Register Addresses
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

typedef struct {
    uint8_t *tx_buffer;    // Pointer to TX buffer
    uint8_t *rx_buffer;    // Pointer to RX buffer
    uint8_t tx_addr[5];    // TX address (5 bytes)
    uint8_t rx_addr[5];    // RX address (5 bytes)
} nrf24l01_device;

// Function prototypes
bool nrf24l01_init(nrf24l01_device *device);
void nrf24l01_set_rx_address(nrf24l01_device *device, const uint8_t *address);
void nrf24l01_set_tx_address(nrf24l01_device *device, const uint8_t *address);
bool nrf24l01_send(nrf24l01_device *device, const uint8_t *data, uint8_t length);
bool nrf24l01_receive(nrf24l01_device *device, uint8_t *data, uint8_t *length);
void nrf24l01_power_up_rx(nrf24l01_device *device);
void nrf24l01_power_up_tx(nrf24l01_device *device);
void nrf24l01_power_down(void);

#endif // NRF24L01_H
