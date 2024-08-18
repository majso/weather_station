#include "nrf24l01.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <string.h>    // For memcpy
#include <stdbool.h> // For bool type
#include <stdio.h>

// Helper function to write a single register
static void nrf24l01_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {NRF24L01_CMD_W_REGISTER | reg, value};
    
    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(100); // Short delay to ensure stable CS
    
    int bytes_written = spi_write_blocking(spi0, buffer, sizeof(buffer));
        
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    sleep_us(100); // Short delay to ensure stable CS
}


// Read a register from the nRF24L01
static uint8_t nrf24l01_read_register(uint8_t reg) {
    uint8_t cmd = NRF24L01_CMD_R_REGISTER | reg;
    uint8_t buffer[2] = {cmd, 0}; // Command + placeholder for read value
    uint8_t status;

    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(100); // Short delay to ensure stable CS

    // Send command and receive status, then read the register value
    spi_write_read_blocking(spi0, buffer, buffer, sizeof(buffer));
    status = buffer[0];
    uint8_t value = buffer[1];

    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    sleep_us(100); // Short delay to ensure stable CS

    return value;
}


// Helper function to write multiple bytes
static void nrf24l01_write_registers(uint8_t reg, const uint8_t *values, uint8_t length) {
    uint8_t buffer[32 + 1]; // Adjust size as needed
    buffer[0] = NRF24L01_CMD_W_REGISTER | reg;
    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(100); // Short delay to ensure stable CS

    memcpy(buffer + 1, values, length);
    spi_write_blocking(spi0, buffer, length + 1);
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    sleep_us(100); // Short delay to ensure stable CS
}

// Helper function to read multiple bytes
static void nrf24l01_read_registers(uint8_t reg, uint8_t *values, uint8_t length) {
    uint8_t buffer[32 + 1]; // Buffer to hold the command and the data
    buffer[0] = NRF24L01_CMD_R_REGISTER | reg; // Command to read from 'reg'

    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low to select the device
    sleep_us(100); // Short delay to ensure stable CS

    // Use spi_write_read_blocking to send the command and receive the data
    spi_write_read_blocking(spi0, buffer, buffer, length + 1);

    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high to deselect the device
    sleep_us(100); // Short delay to ensure stable CS

    memcpy(values, buffer + 1, length); // Copy the received data (excluding the command byte)
}

void nrf24l01_print_config_register_status() {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);

    printf("CONFIG Register: 0x%02X\n", config);
    printf("  PWR_UP: %s\n", (config & (1 << 3)) ? "Power Up" : "Power Down");
    printf("  CRCO: %s\n", (config & (1 << 2)) ? "16-bit CRC" : "8-bit CRC");
    printf("  EN_CRC: %s\n", (config & (1 << 1)) ? "CRC Enabled" : "CRC Disabled");
    printf("  PRIM_RX: %s\n", (config & (1 << 0)) ? "RX Mode" : "TX Mode");
    printf("  Mask Max RT Interrupt: %s\n", (config & (1 << 7)) ? "Masked" : "Not Masked");
    printf("  Mask Data Ready RX FIFO Interrupt: %s\n", (config & (1 << 6)) ? "Masked" : "Not Masked");
    printf("  Mask Data Sent TX FIFO Interrupt: %s\n", (config & (1 << 5)) ? "Masked" : "Not Masked");
}

void nrf24l01_print_register_status() {
    // Read registers
    uint8_t en_aa = nrf24l01_read_register(NRF24L01_REG_EN_AA);
    uint8_t en_rxaddr = nrf24l01_read_register(NRF24L01_REG_EN_RXADDR);
    uint8_t setup_aw = nrf24l01_read_register(NRF24L01_REG_SETUP_AW);
    uint8_t setup_retr = nrf24l01_read_register(NRF24L01_REG_SETUP_RETR);
    uint8_t rf_ch = nrf24l01_read_register(NRF24L01_REG_RF_CH);
    uint8_t rf_setup = nrf24l01_read_register(NRF24L01_REG_RF_SETUP);
    uint8_t status = nrf24l01_read_register(NRF24L01_REG_STATUS);

    // Print status of EN_AA register if any bits are enabled
    if (en_aa != 0x00) {
        printf("EN_AA Register: 0x%02X\n", en_aa);
        if (en_aa & 0x01) printf("  Auto-ACK Pipe 0: Enabled\n");
        if (en_aa & 0x02) printf("  Auto-ACK Pipe 1: Enabled\n");
        if (en_aa & 0x04) printf("  Auto-ACK Pipe 2: Enabled\n");
        if (en_aa & 0x08) printf("  Auto-ACK Pipe 3: Enabled\n");
        if (en_aa & 0x10) printf("  Auto-ACK Pipe 4: Enabled\n");
        if (en_aa & 0x20) printf("  Auto-ACK Pipe 5: Enabled\n");
    }

    // Print status of EN_RXADDR register if any bits are enabled
    if (en_rxaddr != 0x00) {
        printf("EN_RXADDR Register: 0x%02X\n", en_rxaddr);
        if (en_rxaddr & 0x01) printf("  RX Pipe 0: Enabled\n");
        if (en_rxaddr & 0x02) printf("  RX Pipe 1: Enabled\n");
        if (en_rxaddr & 0x04) printf("  RX Pipe 2: Enabled\n");
        if (en_rxaddr & 0x08) printf("  RX Pipe 3: Enabled\n");
        if (en_rxaddr & 0x10) printf("  RX Pipe 4: Enabled\n");
        if (en_rxaddr & 0x20) printf("  RX Pipe 5: Enabled\n");
    }

    // Print status of SETUP_AW register
    printf("SETUP_AW Register: 0x%02X\n", setup_aw);
    printf("  Address Width: %s\n", (setup_aw == 0x03) ? "5 bytes" : "Unknown");

    // Print status of SETUP_RETR register
    printf("SETUP_RETR Register: 0x%02X\n", setup_retr);
    printf("  Auto Retransmit Delay: %dus\n", ((setup_retr & 0x0F) + 1) * 250);
    printf("  Auto Retransmit Count: %d\n", (setup_retr >> 4) & 0x0F);

    // Print status of RF_CH register
    printf("RF_CH Register: 0x%02X\n", rf_ch);
    printf("  Channel: %d\n", rf_ch);

    // Print status of RF_SETUP register
    printf("RF_SETUP Register: 0x%02X\n", rf_setup);
    printf("  Data Rate: %s\n", ((rf_setup >> 2) & 0x05) == 0 ? "1Mbps" :
                                   ((rf_setup >> 2) & 0x05) == 1 ? "250kbps" :
                                   ((rf_setup >> 2) & 0x05) == 4 ? "2Mbps" : "Reserved");
    printf("  Output Power: %d dBm\n", ((rf_setup >> 1) & 0x03) == 0 ? -18 :
                                ((rf_setup >> 1) & 0x03) == 1 ? -12 :
                                   ((rf_setup >> 1) & 0x03) == 2 ? -6 : 0);
    // Print status of STATUS register
    printf("STATUS Register: 0x%02X\n", status);
    printf("  RX_DR: %s\n", (status & (1 << 6)) ? "Data Ready" : "Not Ready");
    printf("  TX_DS: %s\n", (status & (1 << 5)) ? "Data Sent" : "Not Sent");
    printf("  MAX_RT: %s\n", (status & (1 << 4)) ? "Max Retries" : "No Max Retries");
}


bool nrf24l01_init(nrf24l01_device *device) {
    // SPI initialization
    spi_init(spi0, NRF24L01_SPI_SPEED);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(NRF24L01_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(NRF24L01_SPI_MISO_PIN, GPIO_FUNC_SPI);

    gpio_init(NRF24L01_CE_PIN);
    gpio_set_dir(NRF24L01_CE_PIN, GPIO_OUT);
    gpio_put(NRF24L01_CE_PIN, 0);  

    gpio_init(NRF24L01_CS_PIN);
    gpio_set_dir(NRF24L01_CS_PIN, GPIO_OUT);
    gpio_put(NRF24L01_CS_PIN, 1);  

    // Attempt to initialize NRF24L01
    nrf24l01_write_register(NRF24L01_REG_CONFIG, 0x0B); // Power up, CRC 16-bit, RX/TX
    // Check the configuration
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    if (config != 0x0B) {
        printf("Failed to read CONFIG register\n");
        return false; // Initialization failed
    } else {
        printf("Successfully read CONFIG register\n");
    }

    printf("Writting new config to the nRF24L01\n");

    nrf24l01_write_register(NRF24L01_REG_EN_AA, 0x00); // Disable auto-acknowledgment
    nrf24l01_write_register(NRF24L01_REG_EN_RXADDR, 0x01); // Enable RX pipe 0
    nrf24l01_write_register(NRF24L01_REG_SETUP_AW, 0x03); // 5-byte address width
    nrf24l01_write_register(NRF24L01_REG_SETUP_RETR, 0x03); // 1000us, 5 retransmits
    nrf24l01_write_register(NRF24L01_REG_RF_CH, 76); // Channel 76
    nrf24l01_write_register(NRF24L01_REG_RF_SETUP, 0x02); // 1Mbps, 0dBm
    nrf24l01_write_register(NRF24L01_REG_STATUS, 0x70); // Clear interrupts

    // Print the configuration
    nrf24l01_print_config_register_status();
    // Print the status
    nrf24l01_print_register_status();

    printf("Init of nRF24L01 succeeded\n");

    return true; // Initialization succeeded
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
    sleep_ms(2); // Adjust delay as necessary

    // Check if data has been transmitted
    uint8_t status = nrf24l01_read_register(NRF24L01_REG_STATUS);
    return (status & 0x20); // Check if TX_DS (data sent) bit is set
}

bool nrf24l01_receive(nrf24l01_device *device, uint8_t *data, uint8_t *length) {
    gpio_put(NRF24L01_CS_PIN, 0); // Select the NRF24L01
    spi_write_blocking(spi0, (const uint8_t[]){NRF24L01_CMD_R_REGISTER | 0x61}, 1); // Read RX payload
    uint8_t read_buffer[32];
    spi_read_blocking(spi0, 0x00, read_buffer, 32); // Read up to 32 bytes
    gpio_put(NRF24L01_CS_PIN, 1); // Deselect the NRF24L01

    // Copy the data and set length
    memcpy(data, read_buffer, 32);
    *length = 32; // Assuming maximum payload length

    // Check if data has been received
    uint8_t status = nrf24l01_read_register(NRF24L01_REG_STATUS);
    return (status & 0x40); // Check if RX_DR (data ready) bit is set
}

void nrf24l01_power_up_rx(nrf24l01_device *device) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x0F); // Power up and enable RX mode
    gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start RX mode
}

void nrf24l01_power_up_tx(nrf24l01_device *device) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config | 0x0E); // Power up and enable TX mode
    gpio_put(NRF24L01_CE_PIN, 1); // Set CE high to start TX mode
}

void nrf24l01_power_down(void) {
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config & ~0x0E); // Power down
    gpio_put(NRF24L01_CE_PIN, 0); // Set CE low
}