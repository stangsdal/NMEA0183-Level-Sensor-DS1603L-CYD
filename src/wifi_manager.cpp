#include "wifi_manager.h"

// WiFi credentials for AP mode
const char SSID[30] = "UltrasonicSensor";
const char PASSWD[30] = "12345678";

// LED pin for status indication
const byte LED = 26;

// WiFiManager instance
WiFiManager wifiManager;

// Initialize WiFi manager
void wifi_init()
{
    // Setup LED for status indication
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW); // Set LED off initially

    // Set WiFiManager timeout (10 minutes)
    wifiManager.setTimeout(600);
}

// Connect to WiFi using WiFiManager
bool wifi_connect()
{
    // Automatic connection with fallback to setup page
    if (!wifiManager.autoConnect(SSID, PASSWD))
    {
        Serial.println("Failed to connect, shut down WiFi-Modem for 3 Minutes then reset ESP32");

        // Turn off WiFi module
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        delay(180000); // Wait 3 minutes with WiFi module off
        ESP.restart(); // ESP32 uses restart() instead of reset()
        return false;
    }

    // Connection successful
    digitalWrite(LED, HIGH); // Turn on LED (low active)

    return true;
}

// Check if WiFi is connected
bool is_wifi_connected()
{
    return (WiFi.status() == WL_CONNECTED) && (WiFi.localIP() != IPAddress(0, 0, 0, 0));
}

// Check WiFi connection and reset if lost
void wifi_reset_if_lost()
{
    // Only attempt reconnection if WiFi is actually disconnected
    if (!is_wifi_connected())
    {
        Serial.println("WiFi connection lost, attempting to reconnect...");

        // Try to reconnect
        if (!wifiManager.autoConnect(SSID, PASSWD))
        {
            Serial.println("WiFi reconnection failed, restarting ESP32");
            delay(3000);
            ESP.restart();
        }
        else
        {
            digitalWrite(LED, HIGH); // Turn on LED to indicate connection
        }
    }
}

// Get WiFi SSID
String get_wifi_ssid()
{
    return WiFi.SSID();
}

// Get WiFi IP address
IPAddress get_wifi_ip()
{
    return WiFi.localIP();
}

// Get WiFi signal strength
int32_t get_wifi_rssi()
{
    return WiFi.RSSI();
}

// Get WiFi status
wl_status_t get_wifi_status()
{
    return WiFi.status();
}