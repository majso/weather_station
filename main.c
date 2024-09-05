#include "sensors.h"
#include "radio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main()
{
    stdio_init_all();

    printf("Radio starting..\n");
    radio_init(F_433);

    printf("Hello, IoT world from RP2040!\n");
    sensors_init();
    printf("Sensors starting..\n");

    while (true) {
        // Read sensor data
        printf("Reading sensors...\n");
        SensorData sensor_data  = sensors_read_all();
        printf("Reading finished...\n");
        // Send data via radio
        printf("Sending data...\n");
        radio_send_data(&sensor_data);
        printf("Sending finished...\n");
        // Delay between readings
        sleep_ms(10000);
    }
}
