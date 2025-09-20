/*
 * NMEA0183 Level Sensor for ESP32 CYD
 *
 * Description:
 *   Ultrasonic level sensor with NMEA0183 output for marine applications.
 *   Supports ESP32 CYD (Cheap Yellow Display) with LVGL GUI and WiFi connectivity.
 *   Broadcasts sensor data via UDP in NMEA XDR format for integration with
 *   OpenCPN and other marine electronics.
 *
 * Hardware:
 *   - ESP32 CYD (ESP32 with 2.8" ILI9341 TFT display)
 *   - DS1603L ultrasonic distance sensor
 *   - WiFi connectivity for NMEA data broadcast
 *
 * Authors:
 *   - Original concept and implementation: stangsdal
 *   - ESP32 CYD port and modular restructure: Peter Stangsdal
 *
 * License: GNU General Public License v3.0
 *
 * Version History:
 *   - v1.0: Original Wemos D1 Mini implementation
 *   - v2.0: ESP32 CYD port with LVGL GUI and modular architecture
 *
 * Last Modified: September 2025
 *
 * Note: Restructured into modular components for better maintainability.
 *       Each module (display, sensor, wifi_manager, nmea) handles specific
 *       functionality with clean interfaces.
 */

#include <Arduino.h>

// Define TFT settings before including TFT_eSPI
#ifndef TFT_WIDTH
#define TFT_WIDTH 240
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 320
#endif
#ifndef TFT_BL
#define TFT_BL 21
#endif

// Include our modular components
#include "display.h"
#include "sensor.h"
#include "wifi_manager.h"
#include "nmea.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("=== NMEA Level Sensor Starting ===");

    // Initialize display system
    display_init();
    lvgl_init();
    create_ui();

    // Initialize sensor system
    sensor_init();

    // Initialize WiFi system
    wifi_init();
    if (wifi_connect())
    {
        // WiFi connected successfully
    }

    // Initialize NMEA/UDP system
    nmea_init();

    Serial.println("=== System Ready ===");
}
void loop()
{
    static int loop_count = 0;
    loop_count++;

    // Handle LVGL display tasks - this needs to be called regularly
    lv_timer_handler();

    // Read sensor data
    int height_mm = read_sensor();
    calculate_level(height_mm);

    // Get system status
    bool sensor_ok = is_sensor_ok();
    bool wifi_connected = is_wifi_connected();

    // Update status bar
    update_status_bar(wifi_connected, sensor_ok);

    // Check WiFi connection and reset if lost
    wifi_reset_if_lost();

    // Update main display - use the level from calculate_level instead of recalculating
    String level_string = get_level_percent();
    int level_percent = level_string.toInt();

    update_display(height_mm, level_percent, wifi_connected, sensor_ok);

    // Force screen refresh every 20 loops (about once per second)
    if (loop_count % 20 == 0)
    {
        force_screen_refresh();
    }

    // Send NMEA data if WiFi is connected and new sensor data is available
    if (wifi_connected && is_new_data_available())
    {
        String nmea_data = create_nmea_xdr(level_string);
        send_nmea_data(nmea_data);
    }

    // Flash LED to indicate activity
    // Note: LED control is handled in wifi_manager.cpp

    // Small delay to give LVGL time to process and prevent overwhelming the system
    delay(50); // 50ms delay = 20 FPS update rate
}