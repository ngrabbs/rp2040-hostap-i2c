# System Control & Telemetry Subsystem - Pseudocode Flowchart

## Level 0 - High Level Overview

This flowchart represents the wifi_task and webserver_task which together provide
system control via WiFi Access Point and telemetry via HTTP/SSI web interface.

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
                    │   <pico/cyw43_arch> │
                    │   <lwip/apps/httpd> │
                    │   <dhcpserver.h>    │
                    └──────────┬──────────┘
                               │
                        setup() code
                               │
                    ┌──────────┴──────────┐
                    │  Initialize CYW43   │
                    │  WiFi Chip Driver   │
                    └──────────┬──────────┘
                               │
                          ◇────┴────◇
                         ╱           ╲
                        ╱   CYW43     ╲
                       ╱    Init OK?   ╲
                       ╲   (Decision)  ╱
                        ╲             ╱
                         ╲           ╱
                    (false)◇───────◇(true)
                           │       │
                    ┌──────┴───┐   │
                    │  Delete  │   │
                    │  Task    │   │
                    │  (Error) │   │
                    └──────────┘   │
                                   │
                    ┌──────────────┴──────────────┐
                    │   Enable AP Mode            │
                    │   SSID: "Pico2W-AP"         │
                    │   Auth: WPA2-AES-PSK        │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │   Configure Static IP       │
                    │   IP: 192.168.4.1           │
                    │   Netmask: 255.255.255.0    │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │   Initialize DHCP Server    │
                    │   Client IPs: .16 - .23     │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │   Set wifi_ready = true     │
                    │   (Signal to other tasks)   │
                    └──────────────┬──────────────┘
                                   │
                               wifi_task
                              loop() code
                                   │
                    ┌──────────────┴──────────────┐
                    │   Yield to scheduler        │
                    │   vTaskDelay(5000ms)        │
                    │   (CYW43 handles network)   │
                    └──────────────┬──────────────┘
                                   │
                             Loop again
                                   │
═══════════════════════════════════════════════════════
                                   │
                            webserver_task
                          (parallel task)
                                   │
                          ◇────────┴────────◇
                         ╱                   ╲
                        ╱   wifi_ready?       ╲
                       ╱     (Decision)        ╲
                       ╲                       ╱
                        ╲                     ╱
                         ╲                   ╱
                    (false)◇───────────────◇(true)
                           │               │
                    ┌──────┴───┐           │
                    │  Wait    │           │
                    │  100ms   │──┐        │
                    └──────────┘  │        │
                           ▲      │        │
                           └──────┘        │
                                           │
                    ┌──────────────────────┴──────────────────────┐
                    │           Initialize HTTP Server            │
                    │              httpd_init()                   │
                    └──────────────────────┬──────────────────────┘
                                           │
                    ┌──────────────────────┴──────────────────────┐
                    │         Register SSI Handlers               │
                    │   Tags: volt, curr, temp, hum, led, etc.    │
                    └──────────────────────┬──────────────────────┘
                                           │
                    ┌──────────────────────┴──────────────────────┐
                    │         Register CGI Handlers               │
                    │   Endpoints: /led, /config                  │
                    └──────────────────────┬──────────────────────┘
                                           │
                                     loop() code
                                           │
                    ┌──────────────────────┴──────────────────────┐
                    │   Yield to scheduler (1000ms)               │
                    │   (HTTP runs via lwIP callbacks)            │
                    └──────────────────────┬──────────────────────┘
                                           │
                                     Loop again
```

## Flowchart Symbol Legend

| Symbol | Meaning | Color (in PPTX) |
|--------|---------|-----------------|
| Rounded Rectangle | Terminator (Start/End) | Green |
| Rectangle | Process/Statement | Orange |
| Diamond | Decision/Test | Blue |
| Parallelogram | Input/Output | Yellow |
| Arrow | Flow/Path | Black |

## SSI Tags (Server-Side Includes)

The web server dynamically replaces these tags with live data:

| Tag | Description | Source |
|-----|-------------|--------|
| `volt` | Bus voltage (V) | INA238 |
| `curr` | Current (mA) | INA238 |
| `temp` | Temperature (°C) | BME280 |
| `hum` | Humidity (%) | BME280 |
| `led` | LED state (ON/OFF) | GPIO |
| `poll` | Poll interval (ms) | Config |
| `refresh` | Auto-refresh (sec) | Config |
| `ina_ok` | INA238 status | Sensor Task |
| `bme_ok` | BME280 status | Sensor Task |
| `ramfree` | Free heap (bytes) | FreeRTOS |
| `rammin` | Min free heap | FreeRTOS |
| `ramtot` | Total heap | FreeRTOS |

## CGI Endpoints (Control Interface)

| Endpoint | Parameters | Action |
|----------|------------|--------|
| `/led` | `state=on\|off\|toggle` | Control payload LED |
| `/config` | `interval=100-60000` | Set poll interval (ms) |

## Network Configuration

| Parameter | Value |
|-----------|-------|
| SSID | Pico2W-AP |
| Password | password123 |
| Auth | WPA2-AES-PSK |
| AP IP | 192.168.4.1 |
| Netmask | 255.255.255.0 |
| DHCP Range | 192.168.4.16 - .23 |
| Web URL | http://192.168.4.1/ |

## Task Priorities & Resources

| Task | Priority | Stack Size | Role |
|------|----------|------------|------|
| wifi_task | High (3) | 4096 bytes | Network management |
| webserver_task | Medium (2) | 2048 bytes | HTTP/Telemetry |

## Source Files

- `src/tasks/wifi_task.c` - WiFi AP and DHCP server
- `src/tasks/wifi_task.h` - WiFi task API
- `src/tasks/webserver_task.c` - HTTP server with CGI/SSI
- `src/tasks/webserver_task.h` - Web server API
- `src/dhcpserver/dhcpserver.c` - DHCP server implementation
- `src/web/fsdata.c` - Embedded HTML/CSS content
