# Simple RP2030 Weather Station

## Overview

This project aims to build a weather station using the RP2030 microcontroller. The current version includes basic functionality for sensor initialization and data reading. This is an early version, not functional,features are still being developed.

## Planned Features

- **Temperature and Humidity Measurement**: Using SHT30 sensor.
- **Pressure Measurement**: Using BMP280 sensor.
- **Battery and Solar Monitoring**: Using INA219 sensors.
- **Data Logging**: To be added in future versions.

## Current Functionality

The project includes the following functionalities:

- **Sensor Initialization**: Setup for I2C communication and initialization of INA219, SHT30, and BMP280 sensors.
- **Read Sensor Data**:
  - **Temperature**: From SHT30 and BMP280 sensors.
  - **Humidity**: From SHT30 sensor.
  - **Pressure**: From BMP280 sensor.
  - **Battery and Solar Data**: Voltage, current, and power from INA219 sensors.