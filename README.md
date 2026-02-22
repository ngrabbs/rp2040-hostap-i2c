# Pico 2W WiFi Sensor Monitor

A Raspberry Pi Pico 2W project that creates a **WiFi hotspot** you can connect to with your phone or laptop. Once connected, open a web browser to see live sensor readings and control an LED.

---

## Quick Start (For Teammates)

If someone already built the `.uf2` file for you:

1. **Plug in your Pico 2W** while holding the **BOOTSEL** button
2. A drive called **RPI-RP2** will appear on your computer
3. **Drag and drop** the `rp2040-hostap-i2c.uf2` file onto that drive
4. The Pico will reboot automatically
5. **Connect to WiFi** network: `Pico2W-AP` with password: `password123`
6. **Open browser** to: http://192.168.4.1/

That's it! You should see the sensor dashboard.

---

## What This Project Does

```
┌─────────────────────────────────────────────────────────────┐
│                     YOUR PHONE/LAPTOP                       │
│                                                             │
│   1. Connect to WiFi "Pico2W-AP"                           │
│   2. Open http://192.168.4.1/                              │
│   3. See sensor data & control LED                         │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ WiFi
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    PICO 2W (This Project)                   │
│                                                             │
│   - Creates WiFi hotspot (Access Point)                     │
│   - Assigns IP addresses to devices (DHCP)                  │
│   - Runs web server with live sensor data                   │
│   - Reads sensors via I2C every second                      │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ I2C Bus (GPIO 4 & 5)
                            ▼
┌─────────────────────┐     ┌─────────────────────┐
│   INA238 Breakout   │     │   BME280 Breakout   │
│   (Power Monitor)   │     │   (Environment)     │
│                     │     │                     │
│   - Voltage (V)     │     │   - Temperature (C) │
│   - Current (mA)    │     │   - Humidity (%)    │
└─────────────────────┘     └─────────────────────┘
```

---

## Hardware Required

| Part | Description | Where to Get |
|------|-------------|--------------|
| Raspberry Pi Pico 2W | The microcontroller with WiFi | [Adafruit](https://www.adafruit.com/product/5544) |
| Adafruit INA238 | Power/current sensor breakout | [Adafruit](https://www.adafruit.com/product/5832) |
| Adafruit BME280 | Temperature/humidity sensor | [Adafruit](https://www.adafruit.com/product/2652) |
| Jumper Wires | For connections | Any electronics supplier |
| Micro USB Cable | To program and power the Pico | Any USB cable |

---

## Wiring Diagram

Connect everything like this:

```
                    PICO 2W
                 ┌───────────┐
                 │           │
    3.3V Power ──┤ 3V3   GND ├── Ground (all sensors)
                 │           │
         SDA   ──┤ GP4   GP5 ├── SCL
                 │           │
                 └───────────┘
                      │  │
         ┌────────────┴──┴────────────┐
         │                            │
         ▼                            ▼
    ┌─────────┐                  ┌─────────┐
    │ INA238  │                  │ BME280  │
    │         │                  │         │
    │ VIN─3V3 │                  │ VIN─3V3 │
    │ GND─GND │                  │ GND─GND │
    │ SDA─GP4 │                  │ SDA─GP4 │
    │ SCL─GP5 │                  │ SCL─GP5 │
    └─────────┘                  └─────────┘
```

### Pin Connections Table

| Pico 2W Pin | Wire Color (suggested) | Connects To |
|-------------|------------------------|-------------|
| **3V3** (Pin 36) | Red | INA238 VIN, BME280 VIN |
| **GND** (Pin 38) | Black | INA238 GND, BME280 GND |
| **GP4** (Pin 6) | Blue | INA238 SDA, BME280 SDA |
| **GP5** (Pin 7) | Yellow | INA238 SCL, BME280 SCL |

> **Note:** Both sensors share the same I2C bus (same wires). This works because they have different addresses.

---

## Web Interface

Once connected, you'll see a dashboard like this:

```
┌──────────────────────────────────────┐
│     Pico 2W Sensor Monitor           │
├──────────────────────────────────────┤
│ Power Readings (INA238)              │
│   Voltage:     5.02 V                │
│   Current:     125.3 mA              │
│   Status:      OK                    │
├──────────────────────────────────────┤
│ Environment (BME280)                 │
│   Temperature: 23.5 °C               │
│   Humidity:    45.2 %                │
│   Status:      OK                    │
├──────────────────────────────────────┤
│ Payload Control                      │
│   LED Status: OFF                    │
│   [Turn ON]  [Turn OFF]              │
├──────────────────────────────────────┤
│ Settings                             │
│   Poll Interval: [1000] ms  [Set]    │
└──────────────────────────────────────┘
```

The page automatically refreshes every 3 seconds.

---

## Configuration

Default settings (can be changed in `config/app_config.h`):

| Setting | Default Value |
|---------|---------------|
| WiFi Network Name (SSID) | `Pico2W-AP` |
| WiFi Password | `password123` |
| Pico's IP Address | `192.168.4.1` |
| INA238 I2C Address | `0x40` |
| BME280 I2C Address | `0x77` |
| Sensor Poll Rate | 1000 ms (1 second) |

---

## Building From Source

> **Note:** Most teammates can skip this section - just use the pre-built `.uf2` file!

### Prerequisites

You'll need these installed:
- **Pico SDK 2.0+** (with Pico 2W/RP2350 support)
- **ARM GCC Toolchain** (`arm-none-eabi-gcc`)
- **CMake 3.13+**
- **Git**

### Step-by-Step Build Instructions

```bash
# 1. Clone this repository
git clone https://github.com/ngrabbs/rp2040-hostap-i2c.git
cd rp2040-hostap-i2c

# 2. Initialize the FreeRTOS submodule
git submodule update --init --recursive

# 3. Create a build directory
mkdir build
cd build

# 4. Run CMake (adjust PICO_SDK_PATH to your SDK location)
cmake -DPICO_BOARD=pico2_w -DPICO_SDK_PATH=/path/to/pico-sdk ..

# 5. Build the project
make -j4

# 6. The output file is: build/rp2040-hostap-i2c.uf2
```

### Common Build Errors

| Error | Solution |
|-------|----------|
| `PICO_SDK_PATH not set` | Set environment variable: `export PICO_SDK_PATH=/path/to/pico-sdk` |
| `FreeRTOS kernel not found` | Run: `git submodule update --init --recursive` |
| `arm-none-eabi-gcc not found` | Install ARM toolchain for your OS |

---

## Project Structure

```
rp2040-hostap-i2c/
│
├── CMakeLists.txt          # Build configuration
├── pico_sdk_import.cmake   # Pico SDK integration
├── README.md               # This file!
│
├── config/
│   ├── app_config.h        # WiFi, I2C, and app settings
│   └── lwipopts.h          # Network stack configuration
│
├── lib/
│   └── FreeRTOS-Kernel/    # Real-time OS (git submodule)
│
└── src/
    ├── main.c              # Program entry point
    ├── FreeRTOSConfig.h    # FreeRTOS settings
    │
    ├── drivers/            # Sensor drivers
    │   ├── ina238.c/.h     # Power monitor driver
    │   └── bme280.c/.h     # Temp/humidity driver
    │
    ├── tasks/              # FreeRTOS tasks
    │   ├── wifi_task.c/.h      # WiFi access point
    │   ├── sensor_task.c/.h    # Sensor polling
    │   └── webserver_task.c/.h # HTTP server
    │
    └── web/
        └── fsdata.c        # Embedded HTML web page
```

---

## Troubleshooting

### "I can't see the WiFi network"
- Wait 5-10 seconds after plugging in the Pico
- Make sure you're using a **Pico 2W** (has WiFi), not a regular Pico 2
- Try power cycling the Pico (unplug and replug)

### "WiFi connects but web page doesn't load"
- Make sure you're going to `http://192.168.4.1/` (not https)
- Check that your device got an IP address (should be 192.168.4.x)
- Try a different browser

### "Sensors show N/A or FAIL"
- Check your wiring connections
- Make sure sensors are getting 3.3V power
- Verify I2C address matches (INA238: 0x40, BME280: 0x77)

### "LED button doesn't work"
- The LED is the small green one on the Pico 2W board itself
- It's controlled through the WiFi chip, so WiFi must be working

### Need More Help?
- Connect the Pico via USB and open a serial monitor (115200 baud)
- Debug messages will show what's happening

---

## How It Works (For the Curious)

This project uses **FreeRTOS** to run multiple tasks simultaneously:

| Task | Priority | What It Does |
|------|----------|--------------|
| WiFi Task | High | Manages the WiFi access point and DHCP server |
| Webserver Task | Medium | Serves web pages and handles button clicks |
| Sensor Task | Low | Reads I2C sensors at the configured interval |

The tasks communicate through a shared data structure protected by a mutex (to prevent data corruption).

---

## Memory Usage

The Pico 2W has plenty of memory for this project:

| Resource | Used | Available |
|----------|------|-----------|
| RAM | ~275 KB | 520 KB |
| Flash | ~240 KB | 4 MB |

---

## License

MIT License - Feel free to use and modify!

---

## Credits

- Raspberry Pi Pico SDK
- FreeRTOS Project
- lwIP TCP/IP Stack
- Adafruit for great sensor breakout boards
