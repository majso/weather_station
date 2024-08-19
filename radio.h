#ifndef RADIO_H
#define RADIO_H

#include "sensors.h"
#include <stdbool.h>


#define MAX_PAYLOAD_SIZE 32

// Initialize the radio module
void radio_init();

// Send sensor data using the radio module
void radio_send_data(const SensorData *data);
bool radio_receive_data(SensorData *data);
void radio_switch_mode(bool is_transmitting);

#endif // RADIO_H
