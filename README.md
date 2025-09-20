# DS1603L Sensor WiFi NMEA Stream – Documentation

## Quick Start

1. **Connect the hardware**  
   - Install the DS1603L ultrasonic sensor.  
   - Power the Wemos module and ensure WiFi availability.  

2. **First startup**  
   - On first boot (or in an unknown network), a configuration page opens.  
   - Enter your WiFi credentials.  

3. **OpenCPN setup**  
   - In OpenCPN, go to *Connections*.  
   - Add a new **Network Connection**:  
     - Address: `<your-network-broadcast>.255`  
     - Port: `8888`  

4. **Enable Engine Dashboard plugin**  
   - Ensure the *Engine Dashboard* plugin is installed and enabled.  
   - Configure to read the XDR data stream.  

---

## System Behavior

Data is transmitted to the local broadcast address **xxx.xxx.xxx.255** on port **8888**.  

- **Initial startup / unknown network**  
  On first startup, or when connecting to a previously unknown network, the system launches a configuration page with a timeout.  
  If the timeout expires, the WiFi modem is turned off for 3 minutes, after which the Wemos resets and restarts with the configuration page.  

- **Connection loss**  
  If the WiFi connection is lost, the secure login page is displayed.  
  After the timeout, the Wemos resets and re-enters the normal login cycle as described above.  

- **Automatic reconnection**  
  When the WiFi network becomes available again, the device reconnects automatically without requiring a new login.  

---

## Data Transmission

- **Sensor data from the DS1603L ultrasonic sensor is captured and transmitted over WiFi as an NMEA stream via UDP to the network.**  
- All data is broadcast to the network IP ending in `.255` on port **8888**.  
- To improve reliability, four identical packets are sent so that data is correctly received after waking up.  

---

## Sensor Integration

- Sensor values are read every 5 seconds in a dedicated subroutine.  
- Values are stored in a shift register for calculating a **simple moving average**:  
  - The register operates as FIFO (first in, first out).  
  - Each average is based on the last 10 values.  

---

## OpenCPN Integration

- The data stream can be imported into **OpenCPN**.  
- To set this up:  
  1. Create a new **network connection** under *Connections*.  
  2. Use the network’s broadcast address (ending in `.255`) and port **8888**.  
- The system outputs an **XDR sequence**, which can be read using the **Engine Dashboard plugin** in OpenCPN.  
- Different tank types can be supported by adjusting the `FUEL` parameter in the subroutine.  
- For more details, consult the Engine Dashboard plugin documentation.  

---

## System Diagram

```
+-----------------+        +-------------------+        +-----------------+        +------------------+
| DS1603L Sensor  |  UART  |   Wemos (ESP8266) |  WiFi  |   Local Network  |  UDP   |   OpenCPN Host   |
| (Ultrasonic)    +------->+  Firmware (NMEA)  +------->+  Broadcast .255 +------->+ (Engine Dashboard)|
|                 |        |  UDP Port 8888    |        |   (Port 8888)    |        |   Reads XDR       |
+-----------------+        +-------------------+        +-----------------+        +------------------+
          |                           ^
          |                           |
          |          Config / Login (captive portal on first/unknown network)
          |                           |
          v                           |
   Tank level echo           Auto-reconnect on WiFi return
                             (retries; sends 4 packets per sample)
```

**Notes**
- Data format: **NMEA XDR** over **UDP** to the **broadcast IP** (`xxx.xxx.xxx.255`) on **port 8888**.  
- Sampling & smoothing: sensor read every **5 s**, **simple moving average** over **10 samples** (FIFO).  
- Reliability: after wake or reconnect, **four identical packets** are sent to ensure receipt.  
- OpenCPN: add a **Network Connection** → Address: broadcast `.255`, **Port 8888** → read via **Engine Dashboard** plugin (XDR).  
