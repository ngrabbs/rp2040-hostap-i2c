# Power & Distribution Subsystem - Pseudocode Flowchart

## Level 0 - High Level Overview

This flowchart represents the sensor_task in the Power & Distribution subsystem,
which monitors voltage, current, and environmental conditions via I2C sensors.

```
                    ┌─────────────────────┐
                    │        Start        │
                    │    (Terminator)     │
                    └──────────┬──────────┘
                               │
                        global code
                               │
                    ┌──────────┴──────────┐
                    │   Include libraries │
                    │     <ina238.h>      │
                    │     <bme280.h>      │
                    │     <hardware/i2c>  │
                    └──────────┬──────────┘
                               │
                        setup() code
                               │
                    ┌──────────┴──────────┐
                    │   Initialize I2C    │
                    │  GPIO 4 (SDA)       │
                    │  GPIO 5 (SCL)       │
                    │  100 kHz            │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    │  Initialize INA238  │
                    │  Power Monitor      │
                    │  Address: 0x40      │
                    │  Shunt: 15 mOhm     │
                    └──────────┬──────────┘
                               │
                          ◇────┴────◇
                         ╱           ╲
                        ╱   INA238    ╲
                       ╱     OK?       ╲
                       ╲    (Decision) ╱
                        ╲             ╱
                         ╲           ╱
                    (false)◇───────◇(true)
                           │       │
                           │       │
                    ┌──────┴───┐   │
                    │ Mark as  │   │
                    │ INVALID  │   │
                    └──────┬───┘   │
                           │       │
                           └───┬───┘
                               │
                    ┌──────────┴──────────┐
                    │  Initialize BME280  │
                    │  Temp/Humidity      │
                    │  Address: 0x77      │
                    └──────────┬──────────┘
                               │
                          ◇────┴────◇
                         ╱           ╲
                        ╱   BME280    ╲
                       ╱     OK?       ╲
                       ╲    (Decision) ╱
                        ╲             ╱
                         ╲           ╱
                    (false)◇───────◇(true)
                           │       │
                           └───┬───┘
                               │
                        loop() code
                               │
               ┌───────────────┴───────────────┐
               │                               │
               ▼                               ▼
    ┌─────────────────────┐         ┌─────────────────────┐
    │    Read INA238      │         │    Read BME280      │
    │  ┌───────────────┐  │         │  ┌───────────────┐  │
    │  │ Bus Voltage   │  │         │  │ Temperature   │  │
    │  │ Current (mA)  │  │         │  │ Humidity (%)  │  │
    │  │ Power (mW)    │  │         │  └───────────────┘  │
    │  └───────────────┘  │         │     (I/O Block)     │
    │     (I/O Block)     │         └──────────┬──────────┘
    └──────────┬──────────┘                    │
               │                               │
               └───────────────┬───────────────┘
                               │
                    ┌──────────┴──────────┐
                    │  Store readings in  │
                    │   sensor_data_t     │
                    │  (mutex protected)  │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    │  Update heap stats  │
                    │  - free_heap_bytes  │
                    │  - min_free_heap    │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    │ Wait poll_interval  │
                    │   (default 1000ms)  │
                    │  vTaskDelayUntil()  │
                    └──────────┬──────────┘
                               │
                               │
                         Loop again
                               │
                               └──────────────────┐
                                                  │
                               ┌──────────────────┘
                               │
                               ▼
                        (back to loop)
```

## Flowchart Symbol Legend

| Symbol | Meaning | Color (in PPTX) |
|--------|---------|-----------------|
| Rounded Rectangle | Terminator (Start/End) | Green |
| Rectangle | Process/Statement | Orange |
| Diamond | Decision/Test | Blue |
| Parallelogram | Input/Output | Yellow |
| Arrow | Flow/Path | Black |

## Key Data Structures

### sensor_data_t
```c
typedef struct {
    /* INA238 Power Monitor */
    float voltage_v;        // Bus voltage in Volts
    float current_ma;       // Current in milliAmps
    
    /* BME280 Environmental */
    float temperature_c;    // Temperature in Celsius
    float humidity_pct;     // Relative humidity %
    
    /* Status Flags */
    bool ina238_valid;      // INA238 sensor OK
    bool bme280_valid;      // BME280 sensor OK
    
    /* Configuration */
    uint32_t poll_interval_ms;  // Polling rate
    bool led_state;             // Payload LED state
    
    /* System Stats */
    uint32_t free_heap_bytes;
    uint32_t min_free_heap_bytes;
    uint32_t total_heap_bytes;
} sensor_data_t;
```

## Hardware Configuration

| Parameter | Value |
|-----------|-------|
| I2C Port | i2c0 |
| SDA Pin | GPIO 4 |
| SCL Pin | GPIO 5 |
| I2C Speed | 100 kHz |
| INA238 Address | 0x40 |
| BME280 Address | 0x77 |
| Shunt Resistor | 15 mOhm (R015) |
| Max Current | 5.0 A |
| Default Poll | 1000 ms |

## Output Interfaces

The sensor data is made available through:

1. **Web Interface (SSI)** - Auto-refreshing HTML page at http://192.168.4.1/
2. **Serial Console** - Debug output at 115200 baud via USB
3. **Shared Memory** - sensor_data_t struct protected by FreeRTOS mutex

## Source Files

- `src/tasks/sensor_task.c` - Main sensor polling task
- `src/tasks/sensor_task.h` - Data structures and API
- `src/drivers/ina238.c` - INA238 I2C driver
- `src/drivers/bme280.c` - BME280 I2C driver
- `config/app_config.h` - Hardware configuration
