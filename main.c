#include "sensors.h"
#include "radio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
/*
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19 
*/

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9



int main()
{
    stdio_init_all();
    printf("Hello, IoT world from RP2040!\n");
    
    // Initialize sensors and radio
    printf("Sensors starting..\n");
    sensors_init();
    printf("Radio starting..\n");
    radio_init();

    while (true) {
        printf("Ping!\n");

        // Read sensor data
        printf("Reading sensors...\n");
        SensorData data = sensors_read_all();
        printf("Reading finished...\n");
        // Send data via radio
        printf("Sending data...\n");
        radio_send_data(&data);
        printf("Sending finished...\n");
        // Delay between readings
        sleep_ms(1000);
    }
}
