#ifndef RADIO_H
#define RADIO_H

#include <stdbool.h>
#include <stdint.h>

// Init constants
#define F_915       0x00
#define F_433       0x01
#define F_868       0x02

// Register values for different frequencies
// Carrier frequency = 868 MHz
#define F2_868  0x21        
#define F1_868  0x62        
#define F0_868  0x76       
// Carrier frequency = 902 MHz
#define F2_915  0x22        
#define F1_915  0xB1        
#define F0_915  0x3B       
// Carrier frequency = 433 MHz
#define F2_433  0x10       
#define F1_433  0xA7        
#define F0_433  0x62    


typedef struct {
    float temperature;
    float pressure;
    float exterior_temperature;
    float exterior_humidity;
    float battery_voltage;
    float battery_current;
    float battery_power;
    float solar_voltage;
    float solar_current;
    float solar_power;
} SensorData;

// Initialize the radio module
void radio_init(uint8_t f);

// Send sensor data using the radio module
void radio_send_data(const SensorData *data);
void radio_receive_data(SensorData *data);
void radio_switch_mode(bool is_transmitting);

#endif // RADIO_H
