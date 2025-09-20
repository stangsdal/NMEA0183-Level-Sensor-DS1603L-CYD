#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "DS1603L.h"
#include <HardwareSerial.h>
#include <movingAvg.h>

// Function declarations
void sensor_init();
unsigned int read_sensor();
bool is_sensor_ok();
void calculate_level(int height_mm);
String get_level_percent();
int get_raw_height();
bool is_new_data_available();

// Sensor status functions
byte get_sensor_status();
void print_sensor_status();

#endif // SENSOR_H