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
    
    cc1101_reset();
}

void cc1101_write_reg(uint8_t addr, uint8_t value) {
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    
    spi_write_blocking(spi0, &addr, 1);
    spi_write_blocking(spi0, &value, 1);
    
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

void cc1101_write_burst(uint8_t addr, uint8_t* data, uint8_t length) {
    addr |= 0x40;  // Burst mode bit set (bit 6)
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    spi_write_blocking(spi0, &addr, 1);  // Write address with burst mode
    spi_write_blocking(spi0, data, length);  // Write data bytes
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

uint8_t cc1101_read_reg(uint8_t addr) {
    uint8_t result;
    addr |= 0x80; 
    
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    
    spi_write_blocking(spi0, &addr, 1);
    spi_read_blocking(spi0, 0x00, &result, 1);
    
    gpio_put(CC1101_CS_PIN, 1);  // CS high
    
    return result;
}

void cc1101_read_burst(uint8_t addr, uint8_t* buffer, uint8_t length) {
    addr |= 0xC0;  // Burst mode bit set (bit 6) and read bit set (bit 7)
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    spi_write_blocking(spi0, &addr, 1);  // Write address with burst mode
    spi_read_blocking(spi0, 0x00, buffer, length);  // Read data bytes into buffer
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

void cc1101_send_data(uint8_t* data, uint8_t length) {
    // Write data to FIFO
    cc1101_strobe(0x35);  // Strobe TX
    
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    
    uint8_t addr = 0x7F;  // FIFO write address
    spi_write_blocking(spi0, &addr, 1);
    spi_write_blocking(spi0, data, length);
    
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

void cc1101_receive_data(uint8_t* buffer, uint8_t length) {
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    
    uint8_t addr = 0xFF;  // FIFO read address
    spi_write_blocking(spi0, &addr, 1);
    spi_read_blocking(spi0, 0x00, buffer, length);
    
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

void cc1101_strobe(uint8_t strobe) {
    gpio_put(CC1101_CS_PIN, 0);  // CS low
    
    spi_write_blocking(spi0, &strobe, 1);
    
    gpio_put(CC1101_CS_PIN, 1);  // CS high
}

void cc1101_reset(void) {
    gpio_put(CC1101_CS_PIN, 0);
    sleep_ms(10);
    gpio_put(CC1101_CS_PIN, 1);
    sleep_ms(10);
    
    cc1101_strobe(0x30);  // Strobe SRES (reset)
}
