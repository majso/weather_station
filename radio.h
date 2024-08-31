#ifndef RADIO_H
#define RADIO_H

#include "sensors.h"
#include <stdbool.h>

// Initialize the radio module
void radio_init();

// Send sensor data using the radio module
void radio_send_data(const SensorData *data);
void radio_receive_data(SensorData *data);
void radio_switch_mode(bool is_transmitting);

#endif // RADIO_H
