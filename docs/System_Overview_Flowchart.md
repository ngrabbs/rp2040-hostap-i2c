# CubeSat EPS - System Overview Flowchart

## Level 0 - Whole Project Flow

This is a high-level overview of the entire CubeSat EPS monitoring system for team reference.

```
                          ┌─────────────────────┐
                          │       START         │
                          │   (Power On)        │
                          └──────────┬──────────┘
                                     │
                          ┌──────────┴──────────┐
                          │    Initialize HW    │
                          │  • USB Serial       │
                          │  • I2C Bus          │
                          └──────────┬──────────┘
                                     │
              ┌──────────────────────┼──────────────────────┐
              │                      │                      │
              ▼                      ▼                      ▼
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │   WiFi Task     │    │  Sensor Task    │    │ Webserver Task  │
    │  (Priority 2)   │    │  (Priority 1)   │    │  (Priority 1)   │
    └────────┬────────┘    └────────┬────────┘    └────────┬────────┘
             │                      │                      │
             ▼                      ▼                      ▼
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │ Start WiFi AP   │    │ Init INA238     │    │ Wait for WiFi   │
    │ "Pico2W-AP"     │    │ Init BME280     │    │                 │
    └────────┬────────┘    └────────┬────────┘    └────────┬────────┘
             │                      │                      │
             ▼                      │                      ▼
    ┌─────────────────┐             │             ┌─────────────────┐
    │ Start DHCP      │             │             │ Start HTTP      │
    │ Server          │             │             │ Server          │
    │ 192.168.4.x     │             │             │ Port 80         │
    └────────┬────────┘             │             └────────┬────────┘
             │                      │                      │
             ▼                      ▼                      ▼
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │     IDLE        │    │   Poll Loop     │    │  Handle HTTP    │
    │ (handle DHCP)   │    │  (every 1 sec)  │    │   Requests      │
    └─────────────────┘    └────────┬────────┘    └────────┬────────┘
                                    │                      │
                                    ▼                      │
                           ┌───────────────┐               │
                           │  Read Sensors │               │
                           │ ┌───────────┐ │               │
                           │ │  INA238   │ │               │
                           │ │ V, I, P   │ │               │
                           │ ├───────────┤ │               │
                           │ │  BME280   │ │               │
                           │ │ T, H      │ │               │
                           │ └───────────┘ │               │
                           └───────┬───────┘               │
                                   │                       │
                                   ▼                       │
                           ┌───────────────┐               │
                           │ Store in      │◄──────────────┘
                           │ sensor_data_t │  (mutex access)
                           │ (shared)      │───────────────┐
                           └───────┬───────┘               │
                                   │                       │
                                   ▼                       ▼
                           ┌───────────────┐       ┌───────────────┐
                           │ Wait 1 second │       │ SSI: Generate │
                           │ (configurable)│       │ Web Page      │
                           └───────┬───────┘       └───────────────┘
                                   │
                                   └─────────► (loop)
```

---

## System States

| State | Description | Trigger |
|-------|-------------|---------|
| **Init** | Hardware setup, sensor detection | Power-on |
| **Run** | Normal operation, all tasks active | Init complete |
| **Error** | Sensor marked invalid, continue running | Sensor failure |

---

## Data Flow Summary

```
┌──────────────┐         ┌──────────────┐         ┌──────────────┐
│   SENSORS    │         │    SHARED    │         │   OUTPUTS    │
│              │         │    MEMORY    │         │              │
│  INA238 ─────┼────────►│              │────────►│ Web Page     │
│  (V, I, P)   │         │ sensor_data_t│         │ (SSI)        │
│              │         │              │         │              │
│  BME280 ─────┼────────►│  (mutex      │────────►│ Serial       │
│  (T, H)      │         │   protected) │         │ (debug)      │
│              │         │              │         │              │
└──────────────┘         └──────┬───────┘         └──────────────┘
                                │
                                │ CGI
                                ▼
                         ┌──────────────┐
                         │  WEB CLIENT  │
                         │              │
                         │ • Set poll   │
                         │   interval   │
                         │ • Toggle LED │
                         └──────────────┘
```

---

## Quick Reference

| Item | Value |
|------|-------|
| **WiFi SSID** | Pico2W-AP |
| **WiFi Password** | password123 |
| **AP IP Address** | 192.168.4.1 |
| **DHCP Range** | 192.168.4.16-23 |
| **Web URL** | http://192.168.4.1/ |
| **I2C Pins** | GPIO 4 (SDA), GPIO 5 (SCL) |
| **Poll Rate** | 1000 ms (default) |
| **Refresh Rate** | 3 seconds (web page) |

---

## Task Summary

| Task | Priority | Stack | Function |
|------|----------|-------|----------|
| WiFi | 2 (high) | 4 KB | AP + DHCP server |
| Sensor | 1 (normal) | 4 KB | I2C polling |
| Webserver | 1 (normal) | 4 KB | HTTP + SSI/CGI |

---

## Subsystem Responsibilities

| Subsystem | Owner | Key Files |
|-----------|-------|-----------|
| Power Distribution | (You) | `sensor_task.c`, `ina238.c` |
| Environmental | (Shared) | `sensor_task.c`, `bme280.c` |
| System Control | (Shared) | `wifi_task.c`, `webserver_task.c` |
| Telemetry | (Shared) | `webserver_task.c` (SSI/CGI) |
