#include "nmea.h"

// UDP configuration
unsigned int portBroadcast = 8888; // Local port for broadcasting (was 50000 in comments, but code uses 8888)

// WiFiUDP instance
WiFiUDP Udp;

// Data string storage
String XDR1;

// Initialize NMEA/UDP system
void nmea_init()
{
    // Initialize UDP
    if (Udp.begin(portBroadcast))
    {
        // UDP initialized successfully - no verbose logging
    }
    else
    {
        Serial.println("NMEA/UDP system initialization failed");
    }
}

// Calculate checksum for NMEA string
int calculate_checksum(String nmea_string)
{
    int i;
    int XOR;
    int c;

    // Calculate checksum ignoring any $'s in the string
    for (XOR = 0, i = 0; i < 80 && i < nmea_string.length(); i++)
    {
        c = (unsigned char)nmea_string[i];
        if (c == '*')
            break;
        if (c != '$')
            XOR ^= c;
    }
    return XOR;
}

// Create NMEA XDR string
String create_nmea_xdr(String value)
{
    String nmea = "$IIXDR,V,";
    nmea += value;
    nmea += ",P,FUEL*"; // FUEL for fuel tank, adjust for other tank types
    nmea += String(calculate_checksum(nmea), HEX);
    return nmea;
}

// Set data string for transmission
void set_data_string(String nmea_data)
{
    XDR1 = nmea_data;
}

// Get data string
String get_data_string()
{
    return XDR1;
}

// Send NMEA data via UDP broadcast
void send_nmea_data(String nmea_string)
{
    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("UDP send failed: WiFi not connected");
        return;
    }

    // Set broadcast address
    IPAddress broadCast = WiFi.localIP();
    broadCast[3] = 255;

    // Prepare data for transmission
    set_data_string(nmea_string);
    String str = get_data_string();

    // Length (with one extra character for the null terminator)
    int str_len = str.length() + 1;

    // Prepare the character array (the buffer)
    char XDR[str_len];
    str.toCharArray(XDR, str_len);

    // Send single packet with error checking
    if (Udp.beginPacket(broadCast, portBroadcast))
    {
        size_t bytes_written = Udp.write((uint8_t *)XDR, strlen(XDR));
        if (Udp.endPacket())
        {
            // Successful transmission - no verbose logging needed
        }
        else
        {
            Serial.println("UDP send failed: endPacket() failed");
        }
    }
    else
    {
        Serial.println("UDP send failed: beginPacket() failed");
    }
}