# Electrical State Sensing Subsystem - Pseudocode Flowchart

## Level 0 - High Level Overview

This flowchart represents the INA238 power monitor portion of the sensor_task,
which monitors bus voltage, current draw, and power consumption of the system.

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
                    │   <hardware/i2c.h>  │
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
                    │  Verify INA238 ID   │
                    │  Read MFG_ID (0x3E) │
                    │  Read DEV_ID (0x3F) │
                    └──────────┬──────────┘
                               │
                          ◇────┴────◇
                         ╱           ╲
                        ╱  ID Match?  ╲
                       ╱  MFG=0x5449   ╲
                       ╲  DEV=0x238x   ╱
                        ╲ (Decision)  ╱
                         ╲           ╱
                    (false)◇───────◇(true)
                           │       │
                    ┌──────┴───┐   │
                    │ Mark as  │   │
                    │ INVALID  │   │
                    │ ina238_  │   │
                    │ valid=0  │   │
                    └──────┬───┘   │
                           │       │
                           │       ▼
                           │  ┌────────────────────┐
                           │  │   Reset Device     │
                           │  │  Write RST bit to  │
                           │  │  CONFIG (0x00)     │
                           │  └─────────┬──────────┘
                           │            │
                           │  ┌─────────┴──────────┐
                           │  │ Calculate SHUNT_CAL│
                           │  │ Based on:          │
                           │  │ - Shunt: 15 mOhm   │
                           │  │ - Max I: 5.0 A     │
                           │  └─────────┬──────────┘
                           │            │
                           │  ┌─────────┴──────────┐
                           │  │ Write SHUNT_CAL    │
                           │  │ Register (0x02)    │
                           │  │ Value: 0x0753      │
                           │  └─────────┬──────────┘
                           │            │
                           │  ┌─────────┴──────────┐
                           │  │ Configure ADC      │
                           │  │ ADC_CONFIG (0x01)  │
                           │  │ Continuous mode    │
                           │  │ Shunt + Bus + Temp │
                           │  └─────────┬──────────┘
                           │            │
                           └──────┬─────┘
                                  │
                           loop() code
                                  │
                          ◇───────┴───────◇
                         ╱                 ╲
                        ╱  INA238           ╲
                       ╱   Initialized?      ╲
                       ╲    (Decision)       ╱
                        ╲                   ╱
                         ╲                 ╱
                    (false)◇─────────────◇(true)
                           │             │
                    (skip reading)       │
                                         │
                    ┌────────────────────┴────────────────────┐
                    │         Read ADC Registers              │
                    │  ┌───────────────────────────────────┐  │
                    │  │ VBUS (0x05)    - Bus Voltage      │  │
                    │  │ VSHUNT (0x04)  - Shunt Voltage    │  │
                    │  │ CURRENT (0x07) - Calculated I     │  │
                    │  │ POWER (0x08)   - Calculated P     │  │
                    │  │ DIETEMP (0x06) - Die Temperature  │  │
                    │  └───────────────────────────────────┘  │
                    │              (I/O Block)                │
                    └────────────────────┬────────────────────┘
                                         │
                    ┌────────────────────┴────────────────────┐
                    │         Convert Raw Values              │
                    │  ┌───────────────────────────────────┐  │
                    │  │ VBUS:   raw × 3.125mV → Volts    │  │
                    │  │ CURRENT: raw × LSB → mA          │  │
                    │  │ POWER:   raw × LSB × 0.2 → mW    │  │
                    │  └───────────────────────────────────┘  │
                    └────────────────────┬────────────────────┘
                                         │
                          ◇──────────────┴──────────────◇
                         ╱                               ╲
                        ╱        Read Success?            ╲
                       ╱          (Decision)               ╲
                       ╲                                   ╱
                        ╲                                 ╱
                         ╲                               ╱
                    (false)◇───────────────────────────◇(true)
                           │                           │
                    ┌──────┴──────┐             ┌──────┴──────┐
                    │ ina238_     │             │ Store in    │
                    │ valid=false │             │ sensor_data │
                    └──────┬──────┘             │ ┌─────────┐ │
                           │                    │ │voltage_v│ │
                           │                    │ │current_ma│ │
                           │                    │ │ina238_  │ │
                           │                    │ │valid=1  │ │
                           │                    │ └─────────┘ │
                           │                    └──────┬──────┘
                           │                           │
                           └─────────────┬─────────────┘
                                         │
                                   Loop again
                                         │
                    ┌────────────────────┴────────────────────┐
                    │       Output via Web Server             │
                    │  ┌───────────────────────────────────┐  │
                    │  │ SSI Tag: <!--#volt-->  (Voltage)  │  │
                    │  │ SSI Tag: <!--#curr-->  (Current)  │  │
                    │  │ SSI Tag: <!--#ina_ok--> (Status)  │  │
                    │  └───────────────────────────────────┘  │
                    │              (to Telemetry)             │
                    └─────────────────────────────────────────┘
```

## Flowchart Symbol Legend

| Symbol | Meaning | Color (in PPTX) |
|--------|---------|-----------------|
| Rounded Rectangle | Terminator (Start/End) | Green |
| Rectangle | Process/Statement | Orange |
| Diamond | Decision/Test | Blue |
| Parallelogram | Input/Output | Yellow |
| Arrow | Flow/Path | Black |

## INA238 Calibration

The INA238 requires calibration based on the shunt resistor and expected current:

### Shunt Calibration Formula
```
SHUNT_CAL = 819.2 × 10^6 × CURRENT_LSB × R_SHUNT

Where:
  CURRENT_LSB = Max_Current / 2^15
  R_SHUNT = Shunt resistance in Ohms

For this system:
  R_SHUNT = 15 mOhm = 0.015 Ω
  Max_Current = 5.0 A
  CURRENT_LSB = 5.0 / 32768 = 0.0001526 A
  SHUNT_CAL = 819.2e6 × 0.0001526 × 0.015 = 1875 (0x0753)
```

### Conversion Factors
| Register | LSB Value | Units |
|----------|-----------|-------|
| VBUS | 3.125 mV | Volts |
| VSHUNT | 5.0 µV | Volts |
| CURRENT | CURRENT_LSB | Amps |
| POWER | 0.2 × CURRENT_LSB | Watts |
| DIETEMP | 7.8125 m°C | °C |

## Data Structures

### ina238_t (Device Handle)
```c
typedef struct {
    i2c_inst_t *i2c;        // I2C peripheral
    uint8_t addr;           // I2C address (0x40)
    float current_lsb;      // Current LSB in Amps
    float power_lsb;        // Power LSB in Watts
    bool initialized;       // Init status
} ina238_t;
```

### ina238_data_t (Measurement Results)
```c
typedef struct {
    float bus_voltage_v;    // Bus voltage in Volts
    float current_ma;       // Current in milliAmps
    float power_mw;         // Power in milliWatts
    float die_temp_c;       // Die temperature in °C
    bool valid;             // Reading valid flag
} ina238_data_t;
```

## Hardware Configuration

| Parameter | Value |
|-----------|-------|
| Sensor | Texas Instruments INA238 |
| Interface | I2C |
| I2C Address | 0x40 |
| Manufacturer ID | 0x5449 ("TI") |
| Device ID | 0x2381 |
| Shunt Resistor | 15 mOhm (R015) |
| Max Current | 5.0 A |
| SDA Pin | GPIO 4 |
| SCL Pin | GPIO 5 |
| I2C Speed | 100 kHz |

## Measurement Specifications

| Parameter | Range | Resolution |
|-----------|-------|------------|
| Bus Voltage | 0 - 85V | 3.125 mV |
| Shunt Voltage | ±163.84 mV | 5 µV |
| Current | ±5.0 A | 0.15 mA |
| Power | 0 - 425 W | 30 µW |

## INA238 Register Map (Key Registers)

| Address | Name | Description |
|---------|------|-------------|
| 0x00 | CONFIG | Configuration register |
| 0x01 | ADC_CONFIG | ADC settings |
| 0x02 | SHUNT_CAL | Shunt calibration |
| 0x04 | VSHUNT | Shunt voltage (raw) |
| 0x05 | VBUS | Bus voltage (raw) |
| 0x06 | DIETEMP | Die temperature |
| 0x07 | CURRENT | Calculated current |
| 0x08 | POWER | Calculated power |
| 0x3E | MFG_ID | Manufacturer ID |
| 0x3F | DEVICE_ID | Device ID |

## Output Interfaces

Electrical state data is made available through:

1. **Web Interface (SSI Tags)**
   - `<!--#volt-->` - Bus voltage in V
   - `<!--#curr-->` - Current in mA
   - `<!--#ina_ok-->` - Sensor status (OK/FAIL)

2. **Serial Console** - Debug output with raw register values

3. **Shared Memory** - sensor_data_t struct (mutex protected)

## Important Notes

1. **Ground Connection Required**: The INA238 MUST have GND connected to the
   Pico's GND for accurate VBUS (bus voltage) readings. VSHUNT works without
   it because it's a differential measurement.

2. **Shunt Resistor**: The R015 marking indicates 15 mOhm. Incorrect shunt
   value in config will cause proportional current reading errors.

3. **High-Side Sensing**: The INA238 is configured for high-side current
   sensing with VIN+ connected to the power source and VIN- to the load.

## Source Files

- `src/drivers/ina238.c` - INA238 I2C driver
- `src/drivers/ina238.h` - INA238 registers and data structures
- `src/tasks/sensor_task.c` - Polling task (INA238 section)
- `config/app_config.h` - Shunt resistor and max current config
