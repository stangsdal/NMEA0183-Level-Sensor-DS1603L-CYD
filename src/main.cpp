/*
  Es werden Sensorwerte vom Ultraschallsensor DS1603L erfasst und mittels WiFi als NMEA Stream per UDP in das Netzwerk gesendet.
  Die Übertragung erfolgt auf die örtliche Broadcastadresse xxx.xxx.xxx.255 an den Port 50000.

  Nach dem erstmaligen Start oder bei Start in einem Netzwerk welches noch nicht bekannt ist erfolgt der
  Start mit einer Konfigurationsseite und Timeout
  Nach Ablauf von Timeout wird das WiFi-Modem für 3 Minuten abgeschaltet und dann erfolgt reset Wemos und Neustart mit Konfigurationsseite

  Geht die WiFi-Verbindung verloren wird die sichere Anmeldeseite aufgerufen, nach Timeout erfolgt reset Wemos und die normale Anmeldeschleife wie zuvor beschrieben startet.

  Automatische Wiederverbindung bei Wiederkehr WiFi ohne Neuverbindung

  Die Übertragung der Daten erfolgt über NMEA per UDP an BroadcastIP port 50000.
  Es werden 4 Pakete gesendet damit die auch wirklich nach dem aufwachen ankommen.

  Sensoreinbindung
  Die Sensorwerte werden in der Unterroutine alle 5 Sekunden auslesen.
  Die Sensorwerte werden in ein Schieberegister zum gleitenden Durchschnitt gegeben, jeweils 10 Werte bilden Durchschnitt, Register als FIFO (first In / first out)
  Es wird ein einfacher gleitender Durchschnitt gebildet.

  Der Datenstrom kann in OpenCPN eingelesen werden.
  Dazu unter Verbindungen eine neue Netzwerkverbindung einrichten, die Adresse ist die Broadcastadresse des Netzwerks (letzten drei Stellen .255) und der Port ist 50000
  Es wird eine XDR-Sequenz ausgegeben, die dann mit dem Enginedashboard-Plugin in Opencpn ausgelesen werden kann. Verschiedene Tanktypen können durch Anpassen von "FUEL" in der Unterroutine erfasst werden.
  Dazu bitte die Dokumentation vom EngineDashboard-Plugin lesen.


  adapted from Ethernet library examples
  test3
*/

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
// #include <ESP32WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include "DS1603L.h"              //https://github.com/wvmarle/Arduino_DS1603L
#include <HardwareSerial.h>       // ESP32 uses HardwareSerial instead of SoftwareSerial
#include <movingAvg.h>

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

// LVGL and TFT includes
#include <lvgl.h>
#include <TFT_eSPI.h>

// TFT instance
TFT_eSPI tft = TFT_eSPI();

// LVGL display buffer
static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 320;
static lv_color_t buf[screenWidth * 10];
static lv_display_t *disp;

// LVGL UI elements
lv_obj_t *height_label;
lv_obj_t *level_label;
lv_obj_t *wifi_label;
lv_obj_t *sensor_label;

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

// Create the UI
void create_ui()
{
    // Create a main container
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 220, 300);
    lv_obj_center(main_cont);
    
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
    
    // WiFi status
    wifi_label = lv_label_create(main_cont);
    lv_label_set_text(wifi_label, "WiFi: Connecting...");
    lv_obj_align(wifi_label, LV_ALIGN_TOP_MID, 0, 190);
    
    // Sensor status
    sensor_label = lv_label_create(main_cont);
    lv_label_set_text(sensor_label, "Sensor: Initializing...");
    lv_obj_align(sensor_label, LV_ALIGN_TOP_MID, 0, 220);
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
    
    if (wifi_connected) {
        lv_label_set_text(wifi_label, "WiFi: Connected");
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0x00FF00), 0);
    } else {
        lv_label_set_text(wifi_label, "WiFi: Disconnected");
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xFF0000), 0);
    }
    
    if (sensor_ok) {
        lv_label_set_text(sensor_label, "Sensor: OK");
        lv_obj_set_style_text_color(sensor_label, lv_color_hex(0x00FF00), 0);
    } else {
        lv_label_set_text(sensor_label, "Sensor: Error");
        lv_obj_set_style_text_color(sensor_label, lv_color_hex(0xFF0000), 0);
    }
}

//WiFi AP settings
const char SSID[30] = "UltrasonicSensor";
const char PASSWD[30] = "12345678";

//Einstellungen für broadcasting
unsigned int portBroadcast = 8888;      // localer port an den gesendet wird
unsigned int broadCast = 0;

//Variablen für Timer um Sensorwerte zu lesen
unsigned long Timer_RX = 0;
unsigned long Timeout_RX = 2000;         // Intervall in ms hier 5000ms = 5s

// Angabe wie der Sensor angeschlossen ist. ESP32 GPIO pins
const byte txPin = 27;                               // tx of the ESP32 to rx of the sensor (GPIO17)
const byte rxPin = 22;                               // rx of the ESP32 to tx of the sensor (GPIO16)
HardwareSerial sensorSerial(2);                      // Use Serial2 on ESP32

// Indicator LED
const byte LED = 26;                                  // ESP32 LED (moved from pin 2 to avoid conflict with TFT_DC)

// If your sensor is connected to Serial, Serial1, Serial2, AltSoftSerial, etc. pass that object to the sensor constructor.
DS1603L sensor(sensorSerial);

//WiFiUDP
WiFiUDP Udp;

//WiFiManager
WiFiManager wifiManager;

//Variable für Ergebnis Sensorwert
int hoehe = 0;

//Für Berechnungen Prozentwert als Ausgabe
String Fuell = "0";

// Definition eines Arrays von 10 Feldern für gleitenden Durchschnitt
movingAvg filter(10);

// Zum Senden des NMEA-Strings nötig Um die richtige Variable-Form zu bilden
char XDR;
String XDR1;

//Subroutine um Sensorwerte zu bekommen. Sensorwerte werden in ein Schieberegister geschrieben um aus 10 Werten den gleitenden Durchschnitt zu bekommen
unsigned int Sensor () {
  static unsigned int reading = 0;
  if (millis() - Timer_RX > Timeout_RX) {
    Timer_RX = millis ();
    Serial.println(F("Starting reading."));
    reading = uint8_t(sensor.readSensor());           // Call this as often or as little as you want - the sensor transmits every 1-2 seconds.
    byte sensorStatus = sensor.getStatus();           // Check the status of the sensor (not detected; checksum failed; reading success).
    switch (sensorStatus) {                           // For possible values see DS1603L.h
      case DS1603L_NO_SENSOR_DETECTED:                // No sensor detected: no valid transmission received for >10 seconds.
        Serial.println(F("No sensor detected (yet). If no sensor after 1 second, check whether your connections are good."));
        break;
      case DS1603L_READING_SUCCESS:                   // Latest reading was valid and received successfully.
        Serial.print(F("Reading: "));
        Serial.print(reading);
        Serial.println(F(" mm."));
        break;
      case DS1603L_READING_CHECKSUM_FAIL:             // Checksum of the latest transmission failed.
        Serial.print(F("Data received; checksum failed. Latest level reading: "));
        break;
    }
  }
  return filter.reading(reading);
}

//Subroutine zur Ermittlung Höhe und Umrechnung in Prozent Füllung
void Berechnung(int h) {
  int Prozent = (h / 400.00) * 100; //Testrechnung muss an Tank angepasst werden. 400 mm als Gesamthöhe Tank zum Test
  Fuell = String(Prozent, DEC);
  Serial.print("Hight[mm]: ");
  Serial.println(h);
  Serial.print("Level[%]: ");
  Serial.println(Prozent);
  Serial.print("Send: ");
  Serial.println(Fuell);
}

//Calculates the checksum for the NMEA String
int testsum(String strN) {
  int i;
  int XOR;
  int c;
  // Calculate testsum ignoring any $'s in the string
  for (XOR = 0, i = 0; i < 80; i++) {                                    // strlen(strN)
    c = (unsigned char)strN[i];
    if (c == '*') break;
    if (c != '$') XOR ^= c;
  }
  return XOR;
}

//Create NMEA String XDR
String NMEA_XDR(String Val) {
  String nmea = "$IIXDR,V,";
  nmea += Val;
  nmea += ",P,FUEL*"; //FUEL für Treibstoff, Anpassen um weitere Tanktypen zu erfassen, siehe Dokumentation EngineDashboard-Plugin OpenCPN
  nmea += String (testsum(nmea), HEX);
  //nmea += '\r';
  //nmea += '\n';
  return nmea;
}

//Subroutine zur Erstellung Datensatz zum Senden per UDP
void Data(String n) {
  XDR1 = n;
}

//########################################################################

void setup() {

  pinMode(LED, OUTPUT);     // Define LED output
  digitalWrite(LED, LOW);   // Set LED on (low activ)
  
  Serial.begin(115200);
  Serial.println();

  // Initialize TFT display
  tft.init();
  tft.setRotation(0);  // Portrait mode
  tft.fillScreen(TFT_BLACK);
  
  // Turn on backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  // Initialize LVGL
  lvgl_init();
  
  // Create the UI
  create_ui();

  // Initialize Serial2 with specific pins for ESP32
  sensorSerial.begin(9600, SERIAL_8N1, rxPin, txPin); // ESP32 Serial2 with custom pins
  sensor.begin();                                   // Initialise the sensor library.

  //Timeout in sek., nach Ablauf wird die Setup-Seite ausgeschaltet
  wifiManager.setTimeout(600);

  // Average filter
  filter.begin();


  //Automatische Startseite und nach Timeout (wifimanager.setTimeout) erfolgt reset
  if (!wifiManager.autoConnect(SSID, PASSWD)) {  //Gebe eine SSID vor sowie ein Password. Password muss mindestens 7 Zeichen lang sein
    Serial.println("failed to connect, shut down WiFi-Modem for 3 Minutes then reset ESP32");
    //Ausschalten WiFi-Modem (ESP32 equivalent)
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(180000);             //Warte 3 Minuten, in dieser Zeit ist das WiFi-Modul abgeschaltet
    ESP.restart();             // ESP32 uses restart() instead of reset()
  }

  //if you get here you have connected to the WiFi
  digitalWrite(LED, HIGH);   // Set LED on (low activ)
  Serial.println("Connected WiFi successful");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

//########################################################################

void loop() {

  // Handle LVGL tasks
  lv_timer_handler();
  
  hoehe = Sensor();  //Aufruf Subroutine Sensor für Sensorwerte
  Berechnung(hoehe); //Sprung Unterroutine Umrechnung Höhe in % Füllgrad Tank

  // Check sensor status
  bool sensor_ok = (sensor.getStatus() == DS1603L_READING_SUCCESS);
  
  // Update display with current values
  int level_percent = (hoehe / 400.00) * 100; // Same calculation as in Berechnung()
  bool wifi_connected = WiFi.status() == WL_CONNECTED;
  update_display(hoehe, level_percent, wifi_connected, sensor_ok);

  //Überprüfe ob Verbindung zum Netzwerk steht oder starte Setup-Seite
  if (!wifiManager.autoConnect(SSID, PASSWD)) {   //Gebe eine SSID vor sowie ein Password. Password muss mindestens 7 Zeichen lang sein
    Serial.println("WiFi lost, reset ESP32");
    delay(3000);
    ESP.restart();             // ESP32 uses restart() instead of reset()
  }

  //Setze Broadcastadresse
  IPAddress broadCast = WiFi.localIP();
  broadCast[3] = 255;

  //Erstelle Datenstring zum Senden
  Data(NMEA_XDR(Fuell));
  // Wandle den String fürs Senden um
  String str = XDR1;
  //Length (with one extra character for the null terminator)
  int str_len = str.length() + 1;
  // Prepare the character array (the buffer)
  char XDR[str_len];
  // Copy it over
  str.toCharArray(XDR, str_len);

  //Sendeschleife Sende vier Pakete
  for (int i = 0; i < 4; i++) {
    Udp.beginPacket(broadCast, portBroadcast); // send UDP to Port 50000 and BroadcastIP
    Udp.write((uint8_t*)XDR, strlen(XDR));     // ESP32 WiFiUDP needs explicit size
    Udp.endPacket();
  }
  // Flash LED
  digitalWrite(LED, LOW);   // Set LED on (low activ)
  delay(250);
  digitalWrite(LED, HIGH);   // Set LED on (low activ)
}
