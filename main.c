#include "sensors.h"
#include "radio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

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
