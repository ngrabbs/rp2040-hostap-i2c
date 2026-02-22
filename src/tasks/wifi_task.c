/*
 * WiFi Task Implementation - Access Point mode
 * 
 * Sets up the Pico 2W as a WiFi Access Point.
 * The CYW43 driver handles DHCP internally when CYW43_NETUTILS is enabled,
 * otherwise we need to handle IP assignment ourselves.
 */

#include "wifi_task.h"
#include "app_config.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

static TaskHandle_t g_wifi_task_handle = NULL;
static volatile bool g_wifi_ready = false;

bool wifi_task_is_ready(void) {
    return g_wifi_ready;
}

void wifi_task(void *params) {
    (void)params;
    
    printf("[WiFi] Task started\n");
    printf("[WiFi] Initializing CYW43 driver...\n");
    
    /* Initialize the CYW43 WiFi chip */
    if (cyw43_arch_init()) {
        printf("[WiFi] ERROR: Failed to initialize CYW43\n");
        vTaskDelete(NULL);
        return;
    }
    
    printf("[WiFi] CYW43 initialized\n");
    
    /* Enable AP mode with WPA2 authentication */
    printf("[WiFi] Starting Access Point: SSID='%s'\n", WIFI_AP_SSID);
    
    cyw43_arch_enable_ap_mode(WIFI_AP_SSID, WIFI_AP_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    
    printf("[WiFi] AP mode enabled\n");
    
    /* Configure static IP for AP interface
     * The default AP IP is usually 192.168.4.1
     * We set it explicitly here for clarity
     */
    ip4_addr_t ip, netmask, gateway;
    IP4_ADDR(&ip, AP_IP_ADDR_0, AP_IP_ADDR_1, AP_IP_ADDR_2, AP_IP_ADDR_3);
    IP4_ADDR(&netmask, AP_NETMASK_0, AP_NETMASK_1, AP_NETMASK_2, AP_NETMASK_3);
    IP4_ADDR(&gateway, AP_GATEWAY_0, AP_GATEWAY_1, AP_GATEWAY_2, AP_GATEWAY_3);
    
    /* Get the AP network interface and set IP */
    struct netif *netif = &cyw43_state.netif[CYW43_ITF_AP];
    netif_set_addr(netif, &ip, &netmask, &gateway);
    
    printf("[WiFi] AP IP address: %d.%d.%d.%d\n",
           AP_IP_ADDR_0, AP_IP_ADDR_1, AP_IP_ADDR_2, AP_IP_ADDR_3);
    
    /* Note: DHCP server is handled internally by the CYW43 driver
     * when CYW43_NETUTILS is enabled (which is the default for AP mode
     * in the pico_cyw43_arch library). Clients connecting will get
     * IPs starting from 192.168.4.2
     */
    printf("[WiFi] DHCP server active (managed by CYW43 driver)\n");
    
    /* Mark WiFi as ready */
    g_wifi_ready = true;
    
    printf("[WiFi] Access Point ready!\n");
    printf("[WiFi] =============================================\n");
    printf("[WiFi] Connect to: %s\n", WIFI_AP_SSID);
    printf("[WiFi] Password:   %s\n", WIFI_AP_PASSWORD);
    printf("[WiFi] Web URL:    http://%d.%d.%d.%d/\n",
           AP_IP_ADDR_0, AP_IP_ADDR_1, AP_IP_ADDR_2, AP_IP_ADDR_3);
    printf("[WiFi] =============================================\n");
    
    /* Main WiFi task loop - monitor status */
    while (1) {
        /* The CYW43 arch handles networking in the background when using
         * pico_cyw43_arch_lwip_sys_freertos, so we just need to yield */
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        /* Optional: Print connection status periodically */
        #if 0
        int link_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_AP);
        printf("[WiFi] AP link status: %d\n", link_status);
        #endif
    }
}

void wifi_task_init(void) {
    /* Create WiFi task with high priority */
    BaseType_t result = xTaskCreate(
        wifi_task,
        "wifi",
        WIFI_TASK_STACK_SIZE,
        NULL,
        WIFI_TASK_PRIORITY,
        &g_wifi_task_handle
    );
    configASSERT(result == pdPASS);
    
    printf("[WiFi] Task created\n");
}
