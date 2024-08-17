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
    sleep_us(10); // Short delay to ensure stable CS
    
    int bytes_written = spi_write_blocking(spi0, buffer, sizeof(buffer));
    
    if (bytes_written != sizeof(buffer)) {
        printf("spi_write_blocking failed, bytes_written: %d\n", bytes_written);
    }
    
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    printf("Wrote 0x%02X to register 0x%02X\n", value, reg);
}


// Read a register from the nRF24L01
static uint8_t nrf24l01_read_register(uint8_t reg) {
    uint8_t cmd = NRF24L01_CMD_R_REGISTER | reg;
    uint8_t buffer[2] = {cmd, 0}; // Command + placeholder for read value
    uint8_t status;

    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(10); // Short delay to ensure stable CS

    // Send command and receive status, then read the register value
    spi_write_read_blocking(spi0, buffer, buffer, sizeof(buffer));
    status = buffer[0];
    uint8_t value = buffer[1];

    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high

    printf("Status: 0x%02X, Read 0x%02X from register 0x%02X\n", status, value, reg);
    return value;
}


// Helper function to write multiple bytes
static void nrf24l01_write_registers(uint8_t reg, const uint8_t *values, uint8_t length) {
    uint8_t buffer[32 + 1]; // Adjust size as needed
    buffer[0] = NRF24L01_CMD_W_REGISTER | reg;
    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(1); // Add a small delay
    memcpy(buffer + 1, values, length);
    spi_write_blocking(spi0, buffer, length + 1);
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
}

// Helper function to read multiple bytes
static void nrf24l01_read_registers(uint8_t reg, uint8_t *values, uint8_t length) {
    uint8_t buffer[32 + 1]; // Adjust size as needed
    buffer[0] = NRF24L01_CMD_R_REGISTER | reg;
    gpio_put(NRF24L01_CS_PIN, 0); // Set CS low
    sleep_us(1); // Add a small delay
    spi_write_read_blocking(spi0, buffer, buffer, length + 1);
    gpio_put(NRF24L01_CS_PIN, 1); // Set CS high
    memcpy(values, buffer + 1, length);
}

static void nrf24l01_config_PWR_UP() {
    uint8_t mode;
    uint8_t ret;

    // Read the current CONFIG register value
    ret = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    printf("Read CONFIG register: 0x%02X\n", ret); // Add logging statement
    if (ret != 0xFF) { // Check if the read was successful
        mode = ret;

        // Set the PWR_UP bit (bit 1 of CONFIG register)
        mode |= 0x02; // Set bit 1 (PWR_UP)
        
        // Write the modified CONFIG register value back
        nrf24l01_write_register(NRF24L01_REG_CONFIG, mode);

        // Wait for power-up time
        sleep_us(100); // Ensure enough delay for the module to power up
    } else {
        printf("Failed to read CONFIG register\n");
    }
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

    // Ensure the module is powered up
    nrf24l01_config_PWR_UP();
    // Small delay to allow settings to take effect
    sleep_us(100);
    // Attempt to initialize NRF24L01
    nrf24l01_write_register(NRF24L01_REG_CONFIG, 0x0B); // Power up, CRC 16-bit, RX/TX
    sleep_us(100); // Small delay to allow settings to take effect
    // Check the configuration
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    if (config != 0x0B) {
        printf("Failed to read CONFIG register\n");
        return false; // Initialization failed
    }

    printf("Writting config to the nRF24L01\n");

    nrf24l01_write_register(NRF24L01_REG_EN_AA, 0x00); // Disable auto-acknowledgment
    nrf24l01_write_register(NRF24L01_REG_EN_RXADDR, 0x01); // Enable RX pipe 0
    nrf24l01_write_register(NRF24L01_REG_SETUP_AW, 0x03); // 5-byte address width
    nrf24l01_write_register(NRF24L01_REG_SETUP_RETR, 0x03); // 1500us, 5 retransmits
    nrf24l01_write_register(NRF24L01_REG_RF_CH, 76); // Channel 76
    nrf24l01_write_register(NRF24L01_REG_RF_SETUP, 0x0F); // 1Mbps, 0dBm
    nrf24l01_write_register(NRF24L01_REG_STATUS, 0x70); // Clear interrupts

       // Read and display all configuration registers to verify
    printf("Reading all configuration registers\n");
    uint8_t read_buffer[6];
    nrf24l01_read_registers(NRF24L01_REG_CONFIG, read_buffer, sizeof(read_buffer));
    printf("Config register: 0x%02X\n", read_buffer[0]);
    printf("EN_AA register: 0x%02X\n", read_buffer[1]);
    printf("EN_RXADDR register: 0x%02X\n", read_buffer[2]);
    printf("SETUP_AW register: 0x%02X\n", read_buffer[3]);
    printf("SETUP_RETR register: 0x%02X\n", read_buffer[4]);
    printf("RF_CH register: 0x%02X\n", read_buffer[5]);

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