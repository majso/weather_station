#include "sensors.h"
#include "radio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main()
{

    bool is_transmitting = true; // Start as a transmitter

    stdio_init_all();

    printf("Radio starting..\n");
    radio_init();

    printf("Hello, IoT world from RP2040!\n");

    if (is_transmitting) {
        radio_switch_mode(true); // Set to TX mode
        // Initialize sensors and radio
        printf("Sensors starting..\n");
        sensors_init();
    }   

    while (true) {
     if (is_transmitting) {
        radio_switch_mode(true); // Set to TX mode
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
     } else {
            radio_switch_mode(false); // Set to RX mode

            SensorData received_data;
            if (radio_receive_data(&received_data)) {
                printf("Received data:\n");
                printf("  Temperature: %.2f\n", received_data.temperature);
                printf("  Pressure: %.2f\n", received_data.pressure);
                printf("  Exterior Temperature: %.2f\n", received_data.exterior_temperature);
                printf("  Exterior Humidity: %.2f\n", received_data.exterior_humidity);
                printf("  Battery Voltage: %.2f\n", received_data.battery_voltage);
                printf("  Battery Current: %.2f\n", received_data.battery_current);
                printf("  Battery Power: %.2f\n", received_data.battery_power);
                printf("  Solar Voltage: %.2f\n", received_data.solar_voltage);
                printf("  Solar Current: %.2f\n", received_data.solar_current);
                printf("  Solar Power: %.2f\n", received_data.solar_power);
            } else {
                printf("No data received\n");
            }

            sleep_ms(10000); // Wait 10 seconds before switching to TX mode
        }
        //is_transmitting = !is_transmitting; // Toggle mode
    }
}
