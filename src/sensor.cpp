#include "sensor.h"

// Sensor configuration
const byte txPin = 27;                               // tx of the ESP32 to rx of the sensor
const byte rxPin = 22;                               // rx of the ESP32 to tx of the sensor
HardwareSerial sensorSerial(2);                      // Use Serial2 on ESP32

// Sensor instance
DS1603L sensor(sensorSerial);

// Variables for sensor readings
int height = 0;
String Fuell = "0";
bool sensor_initialized = false;
bool new_data_available = false;

// Moving average filter for 10 readings
movingAvg filter(10);

// Timing variables
const unsigned long Timeout_RX = 5000;              // 5 second timeout
unsigned long Timer_RX = 0;

// Initialize sensor
void sensor_init()
{
    // Initialize sensor serial communication
    sensorSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
    
    // Initialize the moving average filter
    filter.begin();
    
    Serial.println("DS1603L sensor initialized");
}

// Read sensor and return filtered value
unsigned int read_sensor()
{
    static unsigned int reading = 0;
    
    if (millis() - Timer_RX > Timeout_RX) {
        Timer_RX = millis();
        
        reading = uint8_t(sensor.readSensor());       // Call this as often or as little as you want
        
        byte sensorStatus = sensor.getStatus();       // Check the status of the sensor
        switch (sensorStatus) {                       // For possible values see DS1603L.h
            case DS1603L_NO_SENSOR_DETECTED:          // No sensor detected
                Serial.println(F("No sensor detected (yet). If no sensor after 1 second, check whether your connections are good."));
                break;
            case DS1603L_READING_SUCCESS:             // Latest reading was valid
                Serial.print(F("Reading: "));
                Serial.print(reading);
                Serial.println(F(" mm."));
                sensor_initialized = true;             // Mark sensor as working
                new_data_available = true;             // Mark new data available
                break;
            case DS1603L_READING_CHECKSUM_FAIL:       // Checksum failed
                Serial.print(F("Data received; checksum failed. Latest level reading: "));
                Serial.print(reading);
                Serial.println(F(" mm."));
                // Still consider sensor as working since we got data
                sensor_initialized = true;
                new_data_available = true;             // Mark new data available (even with checksum fail)
                break;
        }
    }
    
    return filter.reading(reading);
}

// Check if sensor is working properly
bool is_sensor_ok()
{
    // If sensor hasn't been initialized yet, return false
    if (!sensor_initialized) {
        return false;
    }
    
    // Check if current status indicates a working sensor
    byte status = sensor.getStatus();
    return (status == DS1603L_READING_SUCCESS || status == DS1603L_READING_CHECKSUM_FAIL);
}

// Get sensor status byte
byte get_sensor_status()
{
    return sensor.getStatus();
}

// Print sensor status for debugging
void print_sensor_status()
{
    byte status = sensor.getStatus();
    switch (status) {
        case DS1603L_NO_SENSOR_DETECTED:
            Serial.println("Sensor Status: No sensor detected");
            break;
        case DS1603L_READING_SUCCESS:
            Serial.println("Sensor Status: Reading success");
            break;
        case DS1603L_READING_CHECKSUM_FAIL:
            Serial.println("Sensor Status: Checksum failed");
            break;
        default:
            Serial.println("Sensor Status: Unknown");
            break;
    }
}

// Calculate level percentage and update global variables
void calculate_level(int height_mm)
{
    height = height_mm;
    int Prozent = (height_mm / 400.00) * 100; // 400 mm as total tank height for testing
    Fuell = String(Prozent, DEC);
    
   /* Serial.print("Height[mm]: ");
    Serial.println(height_mm);
    Serial.print("Level[%]: ");
    Serial.println(Prozent);
    Serial.print("Send: ");
    Serial.println(Fuell);
    */
}

// Get level percentage as string
String get_level_percent()
{
    return Fuell;
}

// Get raw height reading
int get_raw_height()
{
    return height;
}

// Check if new sensor data is available and clear the flag
bool is_new_data_available()
{
    bool available = new_data_available;
    new_data_available = false;  // Clear the flag after checking
    return available;
}