#include "cc1101.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include "hardware/spi.h"
#include "hardware/gpio.h"

void cc1101_init(void) {
    spi_init(spi0, 500 * 1000);  // 500 kHz SPI
    gpio_set_function(CC1101_SCLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(CC1101_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(CC1101_MISO_PIN, GPIO_FUNC_SPI);
    
    gpio_init(CC1101_CS_PIN);
    gpio_set_dir(CC1101_CS_PIN, GPIO_OUT);
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    gpio_init(CC1101_GDO0_PIN);
    gpio_set_dir(CC1101_GDO0_PIN, GPIO_IN);

    cc1101_reset();
}

void cc1101_write_reg(uint8_t addr, uint8_t value) {
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    sleep_ms(10);
    spi_write_blocking(spi0, &addr, 1);
    spi_write_blocking(spi0, &value, 1);
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    sleep_ms(10);
}

void cc1101_write_burst(uint8_t addr, uint8_t* data, uint8_t length) {
    addr |= 0x40;  // Burst mode bit set (bit 6)
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    sleep_ms(10);
    spi_write_blocking(spi0, &addr, 1);  // Write address with burst mode
    spi_write_blocking(spi0, data, length);  // Write data bytes
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    sleep_ms(10);
}

uint8_t cc1101_read_reg(uint8_t addr) {
    uint8_t result;
    addr |= 0x80; 
    
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    sleep_ms(10);
    spi_write_blocking(spi0, &addr, 1);
    spi_read_blocking(spi0, 0x00, &result, 1);
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    sleep_ms(10);
    
    return result;
}

void cc1101_read_burst(uint8_t addr, uint8_t* buffer, uint8_t length) {
    addr |= 0xC0;  // Burst mode bit set (bit 6) and read bit set (bit 7)
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    sleep_ms(10);
    spi_write_blocking(spi0, &addr, 1);  // Write address with burst mode
    spi_read_blocking(spi0, 0x00, buffer, length);  // Read data bytes into buffer
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    sleep_ms(10);
}

// Function to send data using the TX FIFO
void cc1101_send_data(uint8_t* data, uint8_t length) {
    // Flush TX FIFO before sending data
    cc1101_strobe(CC1101_SFTX);  // SFTX strobe
    // Write data to FIFO
    cc1101_write_burst(CC1101_TXFIFO_BURST, data, length);
     // Set the CC1101 to TX mode to send the data
    cc1101_strobe(CC1101_STX);
}

// Function to receive data using the RX FIFO
void cc1101_receive_data(uint8_t* buffer, uint8_t length) {
    // Flush RX FIFO before receiving data
    cc1101_strobe(CC1101_SFRX);  // SFRX strobe
    // Set the CC1101 to RX mode
    cc1101_strobe(CC1101_SRX);
    // Read the data from the RX FIFO
    cc1101_read_burst(CC1101_RXFIFO_BURST, buffer, length);
}

void cc1101_strobe(uint8_t strobe) {
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    sleep_ms(10);
    spi_write_blocking(spi0, &strobe, 1);
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    sleep_ms(10);
}

void cc1101_reset(void) {
    gpio_put(CC1101_CS_PIN, 0);
    sleep_ms(10);
    gpio_put(CC1101_CS_PIN, 1);
    sleep_ms(10);
    // Reset the CC1101
    cc1101_strobe(CC1101_SRES);  // Strobe SRES (reset)
}
