/*
 * MQTT Module for NMEA0183 Level Sensor
 *
 * Handles MQTT broker connection, reconnection, and data publishing
 * Publishes sensor data to MQTT topics for integration with home automation
 * and marine monitoring systems.
 */

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// MQTT Configuration - adjust these for your MQTT broker
#define MQTT_BROKER_IP "192.168.1.100" // Change to your MQTT broker IP
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "NMEA_Level_Sensor"
#define MQTT_USERNAME "" // Leave empty if no authentication required
#define MQTT_PASSWORD "" // Leave empty if no authentication required

// MQTT Topics
#define MQTT_TOPIC_STATUS "sensors/level/status"
#define MQTT_TOPIC_LEVEL_MM "sensors/level/height_mm"
#define MQTT_TOPIC_LEVEL_PERCENT "sensors/level/percent"
#define MQTT_TOPIC_NMEA_XDR "sensors/level/nmea_xdr"
#define MQTT_TOPIC_WIFI_STATUS "sensors/level/wifi_status"
#define MQTT_TOPIC_SENSOR_STATUS "sensors/level/sensor_status"

// Connection retry configuration
#define MQTT_RECONNECT_INTERVAL 5000 // 5 seconds between reconnection attempts
#define MQTT_MAX_RETRIES 5           // Maximum connection retries before giving up

// Function declarations
void mqtt_init();
bool mqtt_connect();
void mqtt_reconnect();
bool is_mqtt_connected();
void mqtt_loop();
void mqtt_publish_sensor_data(int height_mm, String level_percent);
void mqtt_publish_nmea_data(String nmea_xdr);
void mqtt_publish_status_data(bool wifi_connected, bool sensor_ok);
void mqtt_publish_json_data(int height_mm, String level_percent, bool wifi_connected, bool sensor_ok);

#endif // MQTT_H