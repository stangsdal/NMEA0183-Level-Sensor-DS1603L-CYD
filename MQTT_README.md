# MQTT Configuration Guide

## Quick Setup

### 1. Update MQTT Broker Settings
Edit `src/mqtt.h` and modify these settings for your MQTT broker:

```cpp
// MQTT Configuration - adjust these for your MQTT broker
#define MQTT_BROKER_IP "192.168.1.100"  // Change to your MQTT broker IP
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "NMEA_Level_Sensor"
#define MQTT_USERNAME ""  // Leave empty if no authentication required
#define MQTT_PASSWORD ""  // Leave empty if no authentication required
```

### 2. Popular MQTT Brokers

#### Local Brokers:
- **Mosquitto**: `sudo apt install mosquitto mosquitto-clients`
- **Home Assistant**: Built-in MQTT broker (Settings -> Add-ons -> Mosquitto broker)
- **Docker**: `docker run -it -p 1883:1883 eclipse-mosquitto`

#### Cloud Brokers:
- **HiveMQ**: Free public broker at `broker.hivemq.com`
- **Eclipse IoT**: `mqtt.eclipseprojects.io`

### 3. MQTT Topics Published

The sensor publishes to these topics:

- `sensors/level/status` - JSON status data (retained)
- `sensors/level/height_mm` - Height in millimeters
- `sensors/level/percent` - Level percentage  
- `sensors/level/nmea_xdr` - NMEA XDR string
- `sensors/level/wifi_status` - WiFi connection status
- `sensors/level/sensor_status` - Sensor status

### 4. Testing MQTT

#### Subscribe to all topics:
```bash
mosquitto_sub -h YOUR_BROKER_IP -t "sensors/level/#" -v
```

#### Subscribe to JSON status:
```bash
mosquitto_sub -h YOUR_BROKER_IP -t "sensors/level/status"
```

### 5. Home Assistant Integration

Add to your `configuration.yaml`:

```yaml
sensor:
  - platform: mqtt
    name: "Tank Level"
    state_topic: "sensors/level/percent"
    unit_of_measurement: "%"
    icon: mdi:gauge
    
  - platform: mqtt
    name: "Tank Height"
    state_topic: "sensors/level/height_mm"
    unit_of_measurement: "mm"
    icon: mdi:ruler
    
binary_sensor:
  - platform: mqtt
    name: "Level Sensor Status"
    state_topic: "sensors/level/sensor_status"
    payload_on: "ok"
    payload_off: "error"
    device_class: problem
```

### 6. Node-RED Integration

Import this flow to Node-RED:

```json
[
    {
        "id": "mqtt_level",
        "type": "mqtt in",
        "topic": "sensors/level/status",
        "qos": "2",
        "broker": "your_broker",
        "output": "auto",
        "x": 100,
        "y": 100
    }
]
```

### 7. Troubleshooting

#### Check MQTT connection in Serial Monitor:
- Look for "MQTT connected to broker" message
- Status bar will show "MQTT" in green when connected
- Status bar will show "No MQTT" in red when disconnected

#### Common Issues:
1. **Wrong broker IP**: Update `MQTT_BROKER_IP` in `mqtt.h`
2. **Firewall**: Ensure port 1883 is open
3. **Authentication**: Set username/password if required
4. **WiFi**: MQTT requires WiFi connection first

### 8. Security

For production use:
1. Use TLS/SSL (port 8883)
2. Set username/password authentication
3. Use certificate-based authentication
4. Restrict topic access with ACLs

## Features

- **Automatic reconnection** with exponential backoff
- **Retained status messages** for reliability
- **JSON payload** for comprehensive data
- **Individual topics** for specific data points
- **Display status indicator** shows connection state
- **Configurable retry limits** to prevent endless reconnection attempts