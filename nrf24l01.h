#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include <stdbool.h>

#define NRF24L01_SPI_SPEED    4000000 // 4 MHz, adjust if needed
#define NRF24L01_CS_PIN       10      // Define your Chip Select (CS) pin
#define NRF24L01_CE_PIN       9       // Define your Chip Enable (CE) pin

typedef struct {
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    uint8_t tx_addr[5];
    uint8_t rx_addr[5];
} nrf24l01_device;

void nrf24l01_init(nrf24l01_device *device);
void nrf24l01_set_rx_address(nrf24l01_device *device, const uint8_t *address);
void nrf24l01_set_tx_address(nrf24l01_device *device, const uint8_t *address);
bool nrf24l01_send(nrf24l01_device *device, const uint8_t *data, uint8_t length);
bool nrf24l01_receive(nrf24l01_device *device, uint8_t *data, uint8_t *length);
void nrf24l01_power_up_rx(nrf24l01_device *device);
void nrf24l01_power_up_tx(nrf24l01_device *device);
void nrf24l01_power_down(void);

#endif // NRF24L01_H
