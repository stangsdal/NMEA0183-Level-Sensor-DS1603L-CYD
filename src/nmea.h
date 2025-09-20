#ifndef NMEA_H
#define NMEA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Function declarations
void nmea_init();
String create_nmea_xdr(String value);
int calculate_checksum(String nmea_string);
void send_nmea_data(String nmea_string);
void set_data_string(String nmea_data);
String get_data_string();

// UDP configuration
extern unsigned int portBroadcast;

#endif // NMEA_H