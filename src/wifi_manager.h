#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// Function declarations
void wifi_init();
bool wifi_connect();
bool is_wifi_connected();
void wifi_reset_if_lost();
String get_wifi_ssid();
IPAddress get_wifi_ip();
int32_t get_wifi_rssi();
wl_status_t get_wifi_status();

#endif // WIFI_MANAGER_H