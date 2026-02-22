/*
 * RP2040 HostAP I2C - Main Application
 * 
 * Pico 2W WiFi Access Point with I2C sensor monitoring and web interface.
 * 
 * Features:
 * - WiFi Access Point mode (hostapd equivalent)
 * - DHCP server for connected clients
 * - HTTP web server with live sensor data
 * - INA238 voltage/current monitoring via I2C
 * - BME280 temperature/humidity monitoring via I2C
 * - LED payload control via web interface
 * - Configurable polling interval
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

/* Application headers */
#include "app_config.h"
#include "wifi_task.h"
#include "sensor_task.h"
#include "webserver_task.h"

/* Stack overflow hook for debugging */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    while (1) {
        tight_loop_contents();
    }
}

/* Malloc failed hook for debugging */
void vApplicationMallocFailedHook(void) {
    printf("MALLOC FAILED!\n");
    while (1) {
        tight_loop_contents();
    }
}

int main(void) {
    /* Initialize stdio for debug output over USB */
    stdio_init_all();
    
    /* Wait a bit for USB serial to connect (optional for debugging) */
    sleep_ms(2000);
    
    printf("\n");
    printf("========================================\n");
    printf("   RP2040 HostAP I2C Sensor Monitor\n");
    printf("========================================\n");
    printf("Board: Pico 2W\n");
    printf("WiFi SSID: %s\n", WIFI_AP_SSID);
    printf("AP IP: %d.%d.%d.%d\n", 
           AP_IP_ADDR_0, AP_IP_ADDR_1, AP_IP_ADDR_2, AP_IP_ADDR_3);
    printf("I2C Sensors: INA238 @ 0x%02X, BME280 @ 0x%02X\n",
           INA238_I2C_ADDR, BME280_I2C_ADDR);
    printf("Poll Interval: %d ms\n", DEFAULT_POLL_INTERVAL_MS);
    printf("========================================\n\n");
    
    printf("[Main] Creating FreeRTOS tasks...\n");
    
    /* Create tasks */
    wifi_task_init();
    sensor_task_init();
    webserver_task_init();
    
    printf("[Main] Starting FreeRTOS scheduler...\n\n");
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here */
    printf("[Main] ERROR: Scheduler exited!\n");
    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}
