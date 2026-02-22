/*
 * Application Configuration for RP2040 HostAP I2C Project
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*-----------------------------------------------------------
 * WiFi Access Point Configuration
 *----------------------------------------------------------*/
#define WIFI_AP_SSID            "Pico2W-AP"
#define WIFI_AP_PASSWORD        "password123"
#define WIFI_AP_CHANNEL         6

/* AP IP Configuration */
#define AP_IP_ADDR_0            192
#define AP_IP_ADDR_1            168
#define AP_IP_ADDR_2            4
#define AP_IP_ADDR_3            1

#define AP_NETMASK_0            255
#define AP_NETMASK_1            255
#define AP_NETMASK_2            255
#define AP_NETMASK_3            0

#define AP_GATEWAY_0            192
#define AP_GATEWAY_1            168
#define AP_GATEWAY_2            4
#define AP_GATEWAY_3            1

/*-----------------------------------------------------------
 * I2C Configuration
 *----------------------------------------------------------*/
#define I2C_PORT                i2c0
#define I2C_SDA_PIN             4
#define I2C_SCL_PIN             5
#define I2C_BAUDRATE            100000  /* 100 kHz standard mode */

/*-----------------------------------------------------------
 * Sensor I2C Addresses
 *----------------------------------------------------------*/
#define INA238_I2C_ADDR         0x40
#define BME280_I2C_ADDR         0x77

/*-----------------------------------------------------------
 * INA238 Configuration
 *----------------------------------------------------------*/
/* Shunt resistor value in milliohms */
#define INA238_SHUNT_MOHMS      100     /* 100 mOhm shunt resistor */
#define INA238_MAX_CURRENT_A    3.2     /* Maximum expected current in Amps */

/*-----------------------------------------------------------
 * Sensor Polling Configuration
 *----------------------------------------------------------*/
#define DEFAULT_POLL_INTERVAL_MS    1000    /* Default polling interval */
#define MIN_POLL_INTERVAL_MS        100     /* Minimum allowed interval */
#define MAX_POLL_INTERVAL_MS        60000   /* Maximum allowed interval */

/*-----------------------------------------------------------
 * Payload (LED) Configuration
 *----------------------------------------------------------*/
/* Note: On Pico W/Pico 2W, the onboard LED is connected to the CYW43 chip
 * and is controlled via cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, value) */
#define PAYLOAD_USE_ONBOARD_LED     1

/*-----------------------------------------------------------
 * Web Server Configuration
 *----------------------------------------------------------*/
#define WEB_AUTO_REFRESH_SEC        3       /* Auto-refresh interval in seconds */

/*-----------------------------------------------------------
 * FreeRTOS Task Priorities
 *----------------------------------------------------------*/
#define WIFI_TASK_PRIORITY          (tskIDLE_PRIORITY + 3)
#define WEBSERVER_TASK_PRIORITY     (tskIDLE_PRIORITY + 2)
#define SENSOR_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

/*-----------------------------------------------------------
 * FreeRTOS Task Stack Sizes (in words, not bytes)
 *----------------------------------------------------------*/
#define WIFI_TASK_STACK_SIZE        (4096 / sizeof(StackType_t))
#define WEBSERVER_TASK_STACK_SIZE   (2048 / sizeof(StackType_t))
#define SENSOR_TASK_STACK_SIZE      (1024 / sizeof(StackType_t))

#endif /* APP_CONFIG_H */
