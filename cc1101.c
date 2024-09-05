#include "cc1101.h"
#include "pico/stdlib.h" // For Pico-specific functions like sleep_ms
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

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

// Function to send data using the TX FIFO
void cc1101_send_data(uint8_t* buffer, uint8_t length, uint8_t address) {
    // Write the length byte to TX FIFO
    cc1101_write_reg(CC1101_TXFIFO_SINGLE_BYTE, length);
    // Write the prepared data to TX FIFO
    cc1101_write_burst(CC1101_TXFIFO_BURST, buffer, length);
    // Start the transmission
    cc1101_strobe(CC1101_STX);
    // Wait for GDO0 to be set -> sync transmitted
    while (!gpio_get(CC1101_GDO0_PIN));
    // Wait for GDO0 to be cleared -> end of packet
    while (gpio_get(CC1101_GDO0_PIN));
    // Flush TX FIFO
    cc1101_strobe(CC1101_SFTX);
}
// Function to receive data using the RX FIFOvoid 
void cc1101_receive_data(uint8_t* buffer, uint8_t length) {
    uint8_t rxBytes = 0, rxBytesVerify = 0, marcState = 0;

    // Set to IDLE and flush RX FIFO
    cc1101_strobe(CC1101_SIDLE);
    cc1101_strobe(CC1101_SFRX);
    cc1101_strobe(CC1101_SRX);
    cc1101_signal_strength(); // Print signal strength


    // Recommended process to check RX bytes
    do {
        rxBytes = cc1101_read_reg(CC1101_RXBYTES) & 0x7F;  // Mask to get only the lower 7 bits
        rxBytesVerify = cc1101_read_reg(CC1101_RXBYTES) & 0x7F;
    } while (rxBytes != rxBytesVerify);

    if (rxBytes > 0) {
        marcState = cc1101_read_reg(CC1101_MARCSTATE) & 0x1F;

        // Check for RX FIFO Overflow error
        if (marcState == 0x11) {  // RXFIFO_OVERFLOW
            cc1101_strobe(CC1101_SFRX);  // Clear RX FIFO
            printf("RX FIFO overflow, data discarded.\n");
        } else {
            // Read the RX FIFO content
            cc1101_read_burst(CC1101_RXFIFO_BURST, buffer, rxBytes);

            // Check CRC (bit 7 in the last status byte)
            if (buffer[rxBytes - 1] & 0x80) {
                printf("Packet received correctly.\n");
            } else {
                printf("CRC error, packet discarded.\n");
                memset(buffer, 0, length);  // Clear the buffer due to CRC error
            }
        }
    } else {
        printf("No data in RX FIFO.\n");
    }

    // Set radio back to RX mode
    cc1101_strobe(CC1101_SRX);
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
    // Reset the CC1101
    cc1101_strobe(CC1101_SRES);  // Strobe SRES (reset)
}

void cc1101_signal_strength() {
    uint8_t rssi_raw = cc1101_read_reg(CC1101_RSSI);
    int8_t rssi_dbm;
    if (rssi_raw >= 128) {
        rssi_dbm = (int8_t)(rssi_raw - 256) / 2 - 74;
    } else {
        rssi_dbm = (rssi_raw / 2) - 74;
    }

    printf("Current RSSI: %d dBm\n", rssi_dbm);

    if (rssi_dbm < -100) {
        printf("Signal is very weak\n");
    } else if (rssi_dbm < -70) {
        printf("Signal is moderate\n");
    } else {
        printf("Signal is strong\n");
    }
}