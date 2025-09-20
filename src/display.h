#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Display configuration
extern const uint16_t screenWidth;
extern const uint16_t screenHeight;

// LVGL UI elements
extern lv_obj_t *height_label;
extern lv_obj_t *level_label;
extern lv_obj_t *wifi_label;
extern lv_obj_t *sensor_label;

// Status bar elements
extern lv_obj_t *status_bar;
extern lv_obj_t *wifi_signal_label;
extern lv_obj_t *wifi_status_label;
extern lv_obj_t *sensor_status_label;
extern lv_obj_t *mqtt_status_label;
extern lv_obj_t *time_label;

// Function declarations
void display_init();
void lvgl_init();
void create_status_bar();
void create_ui();
void update_display(int height_mm, int level_percent, bool wifi_connected, bool sensor_ok);
void update_status_bar(bool wifi_connected, bool sensor_ok, bool mqtt_connected);
void update_uptime();
void force_screen_refresh();

// LVGL callback functions
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p);
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data);

#endif // DISPLAY_H