# Power Distribution - Integration Notes

## Team: CubeSat EPS | Subsystem: Power & Distribution

---

## Components

| Component | Type | I2C Address | Purpose |
|-----------|------|-------------|---------|
| INA238 | Power Monitor | 0x40 | Voltage/current sensing |
| BME280 | Environmental | 0x77 | Temperature/humidity |
| 15 mOhm Shunt (R015) | Resistor | N/A | Current measurement |

---

## Pin Assignments

| Pin | Function | Notes |
|-----|----------|-------|
| GPIO 4 | I2C0 SDA | Shared I2C bus |
| GPIO 5 | I2C0 SCL | Shared I2C bus |
| GPIO 0 | LED (CYW43) | Status/payload indicator |

---

## System Behavior

### a) Top-Level States
1. **Init** - Hardware initialization, sensor detection
2. **Monitor** - Continuous polling of sensors
3. **Serve** - Respond to web/telemetry requests

### b) State Transitions
- Power-on → Init → Monitor (automatic)
- Web request → Serve → Monitor (event-driven)
- Sensor failure → Continue with valid flag = false

### c) Power-On Behavior
1. Initialize I2C bus at 100 kHz
2. Detect and configure INA238 (mark invalid if absent)
3. Detect and configure BME280 (mark invalid if absent)
4. Start WiFi AP + DHCP server
5. Begin sensor polling loop

### d) Priority / Control
- **FreeRTOS tasks** enforce isolation (no blocking between subsystems)
- **Mutex** protects shared sensor_data_t struct
- **WiFi task** has higher priority (network responsiveness)
- **Sensor task** runs at normal priority (1 Hz default)

---

## Subsystem Behavior

### Inputs
| Source | Data | Format |
|--------|------|--------|
| INA238 | Bus voltage | float (V) |
| INA238 | Shunt current | float (mA) |
| BME280 | Temperature | float (°C) |
| BME280 | Humidity | float (%) |
| Web CGI | Poll interval | uint32 (ms) |
| Web CGI | LED command | bool |

### Outputs
| Destination | Data | Format |
|-------------|------|--------|
| Web SSI | All sensor values | ASCII strings |
| Serial | Debug messages | 115200 baud |
| Shared struct | sensor_data_t | Binary (mutex protected) |

### Timing
| Operation | Interval | Type |
|-----------|----------|------|
| Sensor poll | 1000 ms (configurable) | Fixed interval |
| Web refresh | 3000 ms | Client-side timer |
| DHCP lease | 300 s | Event-driven |

### Control
- **Error check**: Sensor valid flags checked before use
- **Libraries**: Pico SDK hardware/i2c, FreeRTOS, lwIP
- **Variables**: float for measurements, bool for status

---

## Subsystem Interactions

### Data Flow
| From | To | Data | Push/Pull |
|------|-----|------|-----------|
| Sensor Task | Web Server | sensor_data_t | Pull (via mutex) |
| Web Server | Sensor Task | poll_interval, led_cmd | Push (via CGI) |
| DHCP Server | Clients | IP addresses | Push |

### Data Conversion
- INA238 raw → float V/mA (driver handles)
- BME280 raw → float °C/% (Bosch compensation)
- float → ASCII string (for web SSI)

### Units
| Measurement | Internal | Display |
|-------------|----------|---------|
| Voltage | Volts | V |
| Current | milliAmps | mA |
| Temperature | Celsius | °C |
| Humidity | Percent | % |

### Valid Ranges & Error Handling
| Value | Valid Range | On Breach |
|-------|-------------|-----------|
| Voltage | 0-36 V | Use previous, flag warning |
| Current | 0-5000 mA | Use previous, flag warning |
| Temperature | -40 to +85 °C | Use previous |
| Humidity | 0-100% | Clamp to range |

---

## Project Coordination

### Shared Resources
| Resource | Used By | Protocol |
|----------|---------|----------|
| I2C0 bus | INA238, BME280 | I2C @ 100 kHz |
| WiFi chip | AP, DHCP, HTTP | CYW43 driver |
| FreeRTOS heap | All tasks | ~21 KB used / 160 KB total |

### Data Sharing Plan
- **Global struct**: `sensor_data_t g_sensor_data`
- **Protection**: FreeRTOS mutex (`xSensorMutex`)
- **Access pattern**: Lock → Read/Write → Unlock

### Error Handling
| Error | Prevention | Recovery |
|-------|------------|----------|
| I2C timeout | 100 kHz safe speed | Retry next cycle |
| Sensor absent | Check ID at init | Mark invalid, continue |
| Mutex deadlock | Timeout on lock | Skip update, log warning |

### Debugging
- `printf()` via USB serial (115200 baud)
- LED toggle for heartbeat
- Web page shows raw values + valid flags

---

## Integration Strategy

**Core subsystem**: Power Monitoring (Sensing)

**Rationale**: The device's primary function is monitoring electrical state. All other subsystems (web, telemetry) exist to report this data. The sensor polling loop is the "heartbeat" that drives the system.

---

## Source Files

| File | Purpose |
|------|---------|
| `src/tasks/sensor_task.c` | Main polling loop |
| `src/drivers/ina238.c` | Power monitor driver |
| `src/drivers/bme280.c` | Environmental driver |
| `config/app_config.h` | Hardware constants |
