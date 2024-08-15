#ifndef RADIO_H
#define RADIO_H

#include "sensors.h"

void radio_init(void);
void radio_send_data(const SensorData *data);

#endif