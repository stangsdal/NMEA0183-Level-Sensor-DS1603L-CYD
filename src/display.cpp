#include "display.h"
#include <WiFi.h>

// Display configuration
const uint16_t screenWidth = 240;
const uint16_t screenHeight = 320;

// TFT instance
TFT_eSPI tft = TFT_eSPI();

// LVGL display buffer
static lv_color_t buf[screenWidth * 10]; // Back to original size
static lv_display_t *disp;

// LVGL UI elements
lv_obj_t *height_label;
lv_obj_t *level_label;
lv_obj_t *wifi_label;
lv_obj_t *sensor_label;

// Status bar elements
lv_obj_t *status_bar;
lv_obj_t *wifi_signal_label;
lv_obj_t *wifi_status_label;
lv_obj_t *sensor_status_label;
lv_obj_t *time_label;

// LVGL display flush function
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

// Touch screen support (optional - simplified for now)
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
    // For now, just return no touch
    // TODO: Implement proper touch reading if touch screen is connected
    data->state = LV_INDEV_STATE_REL;
}

// Initialize display hardware
void display_init()
{
    // Initialize TFT
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    // Set backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

// Initialize LVGL display
void lvgl_init()
{
    lv_init();

    // Create display with buffer
    disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Initialize touch input (optional)
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);
}

// Create status bar at top of screen
void create_status_bar()
{
    // Create status bar container
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, screenWidth, 25);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x1F1F1F), 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_set_style_pad_all(status_bar, 2, 0);

    // WiFi signal strength (left side)
    wifi_signal_label = lv_label_create(status_bar);
    lv_label_set_text(wifi_signal_label, "***");
    lv_obj_set_style_text_color(wifi_signal_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(wifi_signal_label, LV_ALIGN_LEFT_MID, 5, 0);

    // WiFi status (left-center)
    wifi_status_label = lv_label_create(status_bar);
    lv_label_set_text(wifi_status_label, "WiFi");
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(wifi_status_label, LV_ALIGN_LEFT_MID, 25, 0);

    // Sensor status (center)
    sensor_status_label = lv_label_create(status_bar);
    lv_label_set_text(sensor_status_label, "ERR");
    lv_obj_set_style_text_color(sensor_status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(sensor_status_label, LV_ALIGN_CENTER, 0, 0);

    // Time/uptime (right side)
    time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(time_label, LV_ALIGN_RIGHT_MID, -5, 0);
}

// Get WiFi signal strength icon
const char *get_wifi_signal_icon(int32_t rssi)
{
    if (rssi > -50)
        return "***"; // Excellent signal (3 bars)
    else if (rssi > -60)
        return "** "; // Good signal (2 bars)
    else if (rssi > -70)
        return "*  "; // Fair signal (1 bar)
    else if (rssi > -80)
        return ".  "; // Weak signal (dot)
    else
        return "X  "; // Very weak/no signal
}

// Get sensor status icon
const char *get_sensor_status_icon(bool sensor_ok)
{
    return sensor_ok ? "OK" : "ERR";
}

// Update status bar
void update_status_bar(bool wifi_connected, bool sensor_ok)
{
    static char time_text[10];

    // Update WiFi status
    if (wifi_connected)
    {
        int32_t rssi = WiFi.RSSI();
        lv_label_set_text(wifi_signal_label, get_wifi_signal_icon(rssi));
        lv_obj_set_style_text_color(wifi_signal_label, lv_color_hex(0x00FF00), 0);

        // Show SSID or "WiFi" if SSID is too long
        String ssid = WiFi.SSID();
        if (ssid.length() > 8)
        {
            lv_label_set_text(wifi_status_label, "WiFi");
        }
        else
        {
            lv_label_set_text(wifi_status_label, ssid.c_str());
        }
        lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(0x00FF00), 0);
    }
    else
    {
        lv_label_set_text(wifi_signal_label, "X  ");
        lv_obj_set_style_text_color(wifi_signal_label, lv_color_hex(0xFF0000), 0);
        lv_label_set_text(wifi_status_label, "No WiFi");
        lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(0xFF0000), 0);
    }

    // Update sensor status
    lv_label_set_text(sensor_status_label, get_sensor_status_icon(sensor_ok));
    if (sensor_ok)
    {
        lv_obj_set_style_text_color(sensor_status_label, lv_color_hex(0x00FF00), 0);
    }
    else
    {
        lv_obj_set_style_text_color(sensor_status_label, lv_color_hex(0xFF0000), 0);
    }

    // Update time (show uptime in minutes:seconds)
    unsigned long uptime = millis() / 1000;
    unsigned long minutes = (uptime / 60) % 60;
    unsigned long seconds = uptime % 60;
    sprintf(time_text, "%02lu:%02lu", minutes, seconds);
    lv_label_set_text(time_label, time_text);
}

// Update uptime display
void update_uptime()
{
    static char time_text[10];
    unsigned long uptime = millis() / 1000;
    unsigned long minutes = (uptime / 60) % 60;
    unsigned long seconds = uptime % 60;
    sprintf(time_text, "%02lu:%02lu", minutes, seconds);
    lv_label_set_text(time_label, time_text);
}

// Create the UI layout
void create_ui()
{
    // First create the status bar at the top
    create_status_bar();

    // Create the main container for content (positioned below status bar)
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 220, 270);            // Reduced height to account for status bar
    lv_obj_align(main_cont, LV_ALIGN_CENTER, 0, 15); // Offset down by status bar height

    // Title
    lv_obj_t *title = lv_label_create(main_cont);
    lv_label_set_text(title, "NMEA Level Sensor");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Height display
    lv_obj_t *height_cont = lv_obj_create(main_cont);
    lv_obj_set_size(height_cont, 200, 60);
    lv_obj_align(height_cont, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *height_title = lv_label_create(height_cont);
    lv_label_set_text(height_title, "Height [mm]:");
    lv_obj_align(height_title, LV_ALIGN_TOP_LEFT, 5, 5);

    height_label = lv_label_create(height_cont);
    lv_label_set_text(height_label, "---");
    lv_obj_set_style_text_font(height_label, &lv_font_montserrat_14, 0);
    lv_obj_align(height_label, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    // Level display
    lv_obj_t *level_cont = lv_obj_create(main_cont);
    lv_obj_set_size(level_cont, 200, 60);
    lv_obj_align(level_cont, LV_ALIGN_TOP_MID, 0, 120);

    lv_obj_t *level_title = lv_label_create(level_cont);
    lv_label_set_text(level_title, "Level [%]:");
    lv_obj_align(level_title, LV_ALIGN_TOP_LEFT, 5, 5);

    level_label = lv_label_create(level_cont);
    lv_label_set_text(level_label, "---");
    lv_obj_set_style_text_font(level_label, &lv_font_montserrat_14, 0);
    lv_obj_align(level_label, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    // Sensor status
    sensor_label = lv_label_create(main_cont);
    lv_label_set_text(sensor_label, "Sensor: Initializing...");
    lv_obj_align(sensor_label, LV_ALIGN_TOP_MID, 0, 200);
}

// Force screen refresh
void force_screen_refresh()
{
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(disp);
}

// Update display with current values
void update_display(int height_mm, int level_percent, bool wifi_connected, bool sensor_ok)
{
    static char height_text[20];
    static char level_text[20];

    sprintf(height_text, "%d", height_mm);
    sprintf(level_text, "%d", level_percent);

    lv_label_set_text(height_label, height_text);
    lv_label_set_text(level_label, level_text);

    // Sensor status with color coding
    if (sensor_ok)
    {
        lv_label_set_text(sensor_label, "Sensor: OK");
        lv_obj_set_style_text_color(sensor_label, lv_color_hex(0x00FF00), 0);
    }
    else
    {
        lv_label_set_text(sensor_label, "Sensor: Error");
        lv_obj_set_style_text_color(sensor_label, lv_color_hex(0xFF0000), 0);
    }

    // Force screen update by invalidating the objects
    lv_obj_invalidate(height_label);
    lv_obj_invalidate(level_label);
    lv_obj_invalidate(sensor_label);
}