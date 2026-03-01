# Environmental Monitoring Subsystem - Pseudocode Flowchart

## Level 0 - High Level Overview

This flowchart represents the BME280 environmental sensor portion of the sensor_task,
which monitors temperature and humidity conditions inside the CubeSat enclosure.

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
                    │     <bme280.h>      │
                    │   <hardware/i2c.h>  │
                    └──────────┬──────────┘
                               │
                        setup() code
                               │
                    ┌──────────┴──────────┐
                    │   Initialize I2C    │
                    │  (shared with INA)  │
                    │  GPIO 4/5, 100kHz   │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    │  Initialize BME280  │
                    │   Address: 0x77     │
                    └──────────┬──────────┘
                               │
                          ◇────┴────◇
                         ╱           ╲
                        ╱  Read Chip  ╲
                       ╱   ID (0x60)?  ╲
                       ╲   (Decision)  ╱
                        ╲             ╱
                         ╲           ╱
                    (false)◇───────◇(true)
                           │       │
                    ┌──────┴───┐   │
                    │ Mark as  │   │
                    │ INVALID  │   │
                    │ bme280_  │   │
                    │ valid=0  │   │
                    └──────┬───┘   │
                           │       │
                           │       ▼
                           │  ┌────────────────────┐
                           │  │ Soft Reset Device  │
                           │  │  Write 0xB6 to     │
                           │  │  reset register    │
                           │  └─────────┬──────────┘
                           │            │
                           │  ┌─────────┴──────────┐
                           │  │ Read Calibration   │
                           │  │ Data (factory)     │
                           │  │ T1-T3, H1-H6       │
                           │  └─────────┬──────────┘
                           │            │
                           │  ┌─────────┴──────────┐
                           │  │ Configure Sensor   │
                           │  │ Oversampling: x1   │
                           │  │ Mode: Normal       │
                           │  └─────────┬──────────┘
                           │            │
                           └──────┬─────┘
                                  │
                           loop() code
                                  │
                    ┌─────────────┴─────────────┐
                    │    Trigger Measurement    │
                    │    (Force mode or         │
                    │     continuous)           │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │   Read Raw ADC Data       │
                    │  ┌─────────────────────┐  │
                    │  │ Temp:  20-bit ADC   │  │
                    │  │ Humid: 16-bit ADC   │  │
                    │  │ Press: 20-bit ADC   │  │
                    │  └─────────────────────┘  │
                    │      (I/O Block)          │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │   Apply Compensation      │
                    │   Formulas (Bosch)        │
                    │  ┌─────────────────────┐  │
                    │  │ Use calibration     │  │
                    │  │ coefficients to     │  │
                    │  │ convert ADC to      │  │
                    │  │ actual values       │  │
                    │  └─────────────────────┘  │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │   Store in sensor_data    │
                    │  ┌─────────────────────┐  │
                    │  │ temperature_c       │  │
                    │  │ humidity_pct        │  │
                    │  │ bme280_valid = true │  │
                    │  └─────────────────────┘  │
                    │   (mutex protected)       │
                    └─────────────┬─────────────┘
                                  │
                            Loop again
                                  │
                    ┌─────────────┴─────────────┐
                    │  Output via Web Server    │
                    │  ┌─────────────────────┐  │
                    │  │ SSI Tag: <!--#temp-->│  │
                    │  │ SSI Tag: <!--#hum--> │  │
                    │  └─────────────────────┘  │
                    │      (to Telemetry)       │
                    └───────────────────────────┘
```

## Flowchart Symbol Legend

| Symbol | Meaning | Color (in PPTX) |
|--------|---------|-----------------|
| Rounded Rectangle | Terminator (Start/End) | Green |
| Rectangle | Process/Statement | Orange |
| Diamond | Decision/Test | Blue |
| Parallelogram | Input/Output | Yellow |
| Arrow | Flow/Path | Black |

## BME280 Compensation Algorithm

The BME280 uses factory-calibrated coefficients stored in ROM. The raw ADC
values must be processed through Bosch's compensation formulas:

### Temperature Compensation
```
Input:  20-bit ADC value (adc_T)
Coefficients: dig_T1, dig_T2, dig_T3
Output: Temperature in 0.01°C units
Side effect: Computes t_fine for humidity calc
```

### Humidity Compensation
```
Input:  16-bit ADC value (adc_H)
Coefficients: dig_H1 through dig_H6
Requires: t_fine from temperature calc
Output: Humidity in 0.01% RH units
```

## Data Structures

### bme280_t (Device Handle)
```c
typedef struct {
    i2c_inst_t *i2c;           // I2C peripheral
    uint8_t addr;              // I2C address (0x77)
    bme280_calib_t calib;      // Calibration data
    int32_t t_fine;            // Fine temp for humidity
    bool initialized;          // Init status
} bme280_t;
```

### bme280_data_t (Measurement Results)
```c
typedef struct {
    float temperature_c;       // Temperature in °C
    float humidity_pct;        // Relative humidity %
    float pressure_pa;         // Pressure in Pa
    bool valid;                // Reading valid flag
} bme280_data_t;
```

## Hardware Configuration

| Parameter | Value |
|-----------|-------|
| Sensor | Bosch BME280 |
| Interface | I2C |
| I2C Address | 0x77 (default) |
| Chip ID | 0x60 |
| Shared I2C Bus | With INA238 |
| SDA Pin | GPIO 4 |
| SCL Pin | GPIO 5 |
| I2C Speed | 100 kHz |

## Measurement Specifications

| Parameter | Range | Resolution | Accuracy |
|-----------|-------|------------|----------|
| Temperature | -40 to +85°C | 0.01°C | ±1.0°C |
| Humidity | 0 to 100% RH | 0.01% | ±3% RH |
| Pressure | 300-1100 hPa | 0.01 hPa | ±1 hPa |

## Output Interfaces

Environmental data is made available through:

1. **Web Interface (SSI Tags)**
   - `<!--#temp-->` - Temperature in °C
   - `<!--#hum-->` - Humidity in %
   - `<!--#bme_ok-->` - Sensor status (OK/FAIL)

2. **Serial Console** - Debug output during init

3. **Shared Memory** - sensor_data_t struct (mutex protected)

## Source Files

- `src/drivers/bme280.c` - BME280 I2C driver with compensation
- `src/drivers/bme280.h` - BME280 registers and data structures
- `src/tasks/sensor_task.c` - Polling task (BME280 section)
