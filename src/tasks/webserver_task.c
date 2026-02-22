/*
 * Webserver Task Implementation - HTTP server with CGI/SSI
 */

#include "webserver_task.h"
#include "wifi_task.h"
#include "sensor_task.h"
#include "app_config.h"
#include "pico/stdlib.h"
#include "lwip/apps/httpd.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static TaskHandle_t g_webserver_task_handle = NULL;

/*
 * SSI (Server-Side Include) Tags
 * These are placeholders in the HTML that get replaced with dynamic values
 */
static const char *ssi_tags[] = {
    "volt",     /* 0: Voltage value */
    "curr",     /* 1: Current value */
    "temp",     /* 2: Temperature value */
    "hum",      /* 3: Humidity value */
    "led",      /* 4: LED state (ON/OFF) */
    "poll",     /* 5: Poll interval */
    "refresh",  /* 6: Auto-refresh interval */
    "ina_ok",   /* 7: INA238 status */
    "bme_ok",   /* 8: BME280 status */
    "ramfree",  /* 9: Free heap bytes */
    "rammin",   /* 10: Minimum ever free heap */
    "ramtot",   /* 11: Total heap size */
};

#define SSI_TAG_COUNT (sizeof(ssi_tags) / sizeof(ssi_tags[0]))

/*
 * SSI Handler - called when SSI tags are found in HTML
 */
static u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    sensor_data_t *data = sensor_task_get_data();
    SemaphoreHandle_t mutex = sensor_task_get_mutex();
    
    /* Default to empty string */
    pcInsert[0] = '\0';
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        snprintf(pcInsert, iInsertLen, "ERR");
        return strlen(pcInsert);
    }
    
    switch (iIndex) {
        case 0: /* volt */
            if (data->ina238_valid) {
                snprintf(pcInsert, iInsertLen, "%.2f", data->voltage_v);
            } else {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            break;
            
        case 1: /* curr */
            if (data->ina238_valid) {
                snprintf(pcInsert, iInsertLen, "%.1f", data->current_ma);
            } else {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            break;
            
        case 2: /* temp */
            if (data->bme280_valid) {
                snprintf(pcInsert, iInsertLen, "%.1f", data->temperature_c);
            } else {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            break;
            
        case 3: /* hum */
            if (data->bme280_valid) {
                snprintf(pcInsert, iInsertLen, "%.1f", data->humidity_pct);
            } else {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            break;
            
        case 4: /* led */
            snprintf(pcInsert, iInsertLen, "%s", data->led_state ? "ON" : "OFF");
            break;
            
        case 5: /* poll */
            snprintf(pcInsert, iInsertLen, "%lu", (unsigned long)data->poll_interval_ms);
            break;
            
        case 6: /* refresh */
            snprintf(pcInsert, iInsertLen, "%d", WEB_AUTO_REFRESH_SEC);
            break;
            
        case 7: /* ina_ok */
            snprintf(pcInsert, iInsertLen, "%s", data->ina238_valid ? "OK" : "FAIL");
            break;
            
        case 8: /* bme_ok */
            snprintf(pcInsert, iInsertLen, "%s", data->bme280_valid ? "OK" : "FAIL");
            break;
            
        case 9: /* ramfree */
            snprintf(pcInsert, iInsertLen, "%lu", (unsigned long)data->free_heap_bytes);
            break;
            
        case 10: /* rammin */
            snprintf(pcInsert, iInsertLen, "%lu", (unsigned long)data->min_free_heap_bytes);
            break;
            
        case 11: /* ramtot */
            snprintf(pcInsert, iInsertLen, "%lu", (unsigned long)data->total_heap_bytes);
            break;
            
        default:
            snprintf(pcInsert, iInsertLen, "???");
            break;
    }
    
    xSemaphoreGive(mutex);
    
    return strlen(pcInsert);
}

/*
 * CGI Handlers - handle form submissions and button clicks
 */

/* LED control CGI handler */
static const char *led_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    (void)iIndex;
    
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "state") == 0) {
            if (strcmp(pcValue[i], "on") == 0 || strcmp(pcValue[i], "1") == 0) {
                sensor_task_set_led(true);
                printf("[Web] LED turned ON\n");
            } else if (strcmp(pcValue[i], "off") == 0 || strcmp(pcValue[i], "0") == 0) {
                sensor_task_set_led(false);
                printf("[Web] LED turned OFF\n");
            } else if (strcmp(pcValue[i], "toggle") == 0) {
                bool current = sensor_task_get_led();
                sensor_task_set_led(!current);
                printf("[Web] LED toggled to %s\n", !current ? "ON" : "OFF");
            }
            break;
        }
    }
    
    /* Redirect back to main page */
    return "/index.shtml";
}

/* Poll interval configuration CGI handler */
static const char *config_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    (void)iIndex;
    
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "interval") == 0) {
            uint32_t interval = (uint32_t)atoi(pcValue[i]);
            if (interval >= MIN_POLL_INTERVAL_MS && interval <= MAX_POLL_INTERVAL_MS) {
                sensor_task_set_poll_interval(interval);
                printf("[Web] Poll interval set to %lu ms\n", (unsigned long)interval);
            } else {
                printf("[Web] Invalid poll interval: %lu ms\n", (unsigned long)interval);
            }
            break;
        }
    }
    
    /* Redirect back to main page */
    return "/index.shtml";
}

/* CGI handler table */
static const tCGI cgi_handlers[] = {
    {"/led", led_cgi_handler},
    {"/led.cgi", led_cgi_handler},
    {"/config", config_cgi_handler},
    {"/config.cgi", config_cgi_handler},
};

#define CGI_HANDLER_COUNT (sizeof(cgi_handlers) / sizeof(cgi_handlers[0]))

void webserver_task(void *params) {
    (void)params;
    
    printf("[Web] Task started, waiting for WiFi...\n");
    
    /* Wait for WiFi to be ready */
    while (!wifi_task_is_ready()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    printf("[Web] WiFi ready, initializing HTTP server...\n");
    
    /* Initialize the HTTP server */
    httpd_init();
    
    /* Register SSI handlers */
    http_set_ssi_handler(ssi_handler, ssi_tags, SSI_TAG_COUNT);
    
    /* Register CGI handlers */
    http_set_cgi_handlers(cgi_handlers, CGI_HANDLER_COUNT);
    
    printf("[Web] HTTP server started\n");
    printf("[Web] SSI tags registered: %d\n", SSI_TAG_COUNT);
    printf("[Web] CGI handlers registered: %d\n", CGI_HANDLER_COUNT);
    
    /* Main webserver task loop */
    while (1) {
        /* HTTP server runs via lwIP callbacks, we just yield */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void webserver_task_init(void) {
    /* Create webserver task */
    BaseType_t result = xTaskCreate(
        webserver_task,
        "web",
        WEBSERVER_TASK_STACK_SIZE,
        NULL,
        WEBSERVER_TASK_PRIORITY,
        &g_webserver_task_handle
    );
    configASSERT(result == pdPASS);
    
    printf("[Web] Task created\n");
}
