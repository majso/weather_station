#ifndef RADIO_H
#define RADIO_H

#include "sensors.h"

// Initialize the radio module
void radio_init();

// Send sensor data using the radio module
void radio_send_data(const SensorData *data);

#endif // RADIO_H
