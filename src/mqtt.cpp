#include "mqtt.h"

// WiFi and MQTT client instances
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Connection state tracking
bool mqtt_initialized = false;
unsigned long last_reconnect_attempt = 0;
int connection_retries = 0;

// Initialize MQTT system
void mqtt_init()
{
    // Configure MQTT broker
    mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);

    // Set keep alive and socket timeout for better reliability
    mqttClient.setKeepAlive(60); // 60 seconds keep alive

    mqtt_initialized = true;
    Serial.println("MQTT system initialized");
}

// Connect to MQTT broker
bool mqtt_connect()
{
    if (!mqtt_initialized)
    {
        Serial.println("MQTT not initialized");
        return false;
    }

    if (!WiFi.isConnected())
    {
        Serial.println("WiFi not connected, cannot connect to MQTT");
        return false;
    }

    // Attempt connection
    bool connected = false;

    if (strlen(MQTT_USERNAME) > 0 && strlen(MQTT_PASSWORD) > 0)
    {
        // Connect with authentication
        connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    }
    else
    {
        // Connect without authentication
        connected = mqttClient.connect(MQTT_CLIENT_ID);
    }

    if (connected)
    {
        Serial.print("MQTT connected to broker: ");
        Serial.println(MQTT_BROKER_IP);

        // Publish initial status
        mqttClient.publish(MQTT_TOPIC_STATUS, "online", true); // Retained message

        // Reset retry counter
        connection_retries = 0;

        return true;
    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(mqttClient.state());
        return false;
    }
}

// Handle MQTT reconnection
void mqtt_reconnect()
{
    if (!mqtt_initialized || !WiFi.isConnected())
    {
        return;
    }

    unsigned long now = millis();

    // Check if it's time to attempt reconnection
    if (now - last_reconnect_attempt > MQTT_RECONNECT_INTERVAL)
    {
        last_reconnect_attempt = now;

        // Don't retry indefinitely
        if (connection_retries < MQTT_MAX_RETRIES)
        {
            Serial.print("Attempting MQTT connection... ");

            if (mqtt_connect())
            {
                Serial.println("connected");
            }
            else
            {
                connection_retries++;
                Serial.print("failed, retry ");
                Serial.print(connection_retries);
                Serial.print("/");
                Serial.println(MQTT_MAX_RETRIES);
            }
        }
        else
        {
            // Reset retry counter after a longer delay
            if (now - last_reconnect_attempt > (MQTT_RECONNECT_INTERVAL * 10))
            {
                connection_retries = 0;
                Serial.println("Resetting MQTT retry counter");
            }
        }
    }
}

// Check if MQTT is connected
bool is_mqtt_connected()
{
    return mqtt_initialized && mqttClient.connected();
}

// MQTT loop handler - must be called regularly
void mqtt_loop()
{
    if (!mqtt_initialized)
    {
        return;
    }

    if (mqttClient.connected())
    {
        mqttClient.loop(); // Handle MQTT client tasks
    }
    else
    {
        mqtt_reconnect(); // Attempt reconnection if disconnected
    }
}

// Publish sensor data to individual topics
void mqtt_publish_sensor_data(int height_mm, String level_percent)
{
    if (!is_mqtt_connected())
    {
        return;
    }

    // Publish height in millimeters
    String height_str = String(height_mm);
    mqttClient.publish(MQTT_TOPIC_LEVEL_MM, height_str.c_str());

    // Publish level percentage
    mqttClient.publish(MQTT_TOPIC_LEVEL_PERCENT, level_percent.c_str());
}

// Publish NMEA XDR data
void mqtt_publish_nmea_data(String nmea_xdr)
{
    if (!is_mqtt_connected())
    {
        return;
    }

    mqttClient.publish(MQTT_TOPIC_NMEA_XDR, nmea_xdr.c_str());
}

// Publish system status data
void mqtt_publish_status_data(bool wifi_connected, bool sensor_ok)
{
    if (!is_mqtt_connected())
    {
        return;
    }

    // Publish WiFi status
    mqttClient.publish(MQTT_TOPIC_WIFI_STATUS, wifi_connected ? "connected" : "disconnected");

    // Publish sensor status
    mqttClient.publish(MQTT_TOPIC_SENSOR_STATUS, sensor_ok ? "ok" : "error");
}

// Publish comprehensive JSON data (useful for home automation systems)
void mqtt_publish_json_data(int height_mm, String level_percent, bool wifi_connected, bool sensor_ok)
{
    if (!is_mqtt_connected())
    {
        return;
    }

    // Create JSON payload
    String json = "{";
    json += "\"height_mm\":" + String(height_mm) + ",";
    json += "\"level_percent\":" + level_percent + ",";
    json += "\"wifi_connected\":" + String(wifi_connected ? "true" : "false") + ",";
    json += "\"sensor_ok\":" + String(sensor_ok ? "true" : "false") + ",";
    json += "\"timestamp\":" + String(millis()) + ",";
    json += "\"client_id\":\"" + String(MQTT_CLIENT_ID) + "\"";
    json += "}";

    // Publish to status topic
    mqttClient.publish(MQTT_TOPIC_STATUS, json.c_str());
}