/*
 * Sensor Task Implementation
 */

#include "sensor_task.h"
#include "app_config.h"
#include "ina238.h"
#include "bme280.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

/* Static sensor data and mutex */
static sensor_data_t g_sensor_data = {0};
static SemaphoreHandle_t g_sensor_mutex = NULL;
static TaskHandle_t g_sensor_task_handle = NULL;

/* Sensor device handles */
static ina238_t g_ina238;
static bme280_t g_bme280;

sensor_data_t* sensor_task_get_data(void) {
    return &g_sensor_data;
}

SemaphoreHandle_t sensor_task_get_mutex(void) {
    return g_sensor_mutex;
}

void sensor_task_set_poll_interval(uint32_t interval_ms) {
    if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        /* Clamp to valid range */
        if (interval_ms < MIN_POLL_INTERVAL_MS) {
            interval_ms = MIN_POLL_INTERVAL_MS;
        }
        if (interval_ms > MAX_POLL_INTERVAL_MS) {
            interval_ms = MAX_POLL_INTERVAL_MS;
        }
        g_sensor_data.poll_interval_ms = interval_ms;
        xSemaphoreGive(g_sensor_mutex);
    }
}

void sensor_task_set_led(bool state) {
    if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_sensor_data.led_state = state;
        xSemaphoreGive(g_sensor_mutex);
    }
    
    /* Update LED hardware */
#if PAYLOAD_USE_ONBOARD_LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state ? 1 : 0);
#endif
}

bool sensor_task_get_led(void) {
    bool state = false;
    if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        state = g_sensor_data.led_state;
        xSemaphoreGive(g_sensor_mutex);
    }
    return state;
}

static void init_i2c(void) {
    /* Initialize I2C peripheral */
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    printf("[Sensor] I2C initialized on GPIO %d (SDA), GPIO %d (SCL) at %d Hz\n",
           I2C_SDA_PIN, I2C_SCL_PIN, I2C_BAUDRATE);
}

static void init_sensors(void) {
    /* Initialize INA238 */
    printf("[Sensor] Initializing INA238 at address 0x%02X...\n", INA238_I2C_ADDR);
    if (ina238_init(&g_ina238, I2C_PORT, INA238_I2C_ADDR, 
                    INA238_SHUNT_MOHMS, INA238_MAX_CURRENT_A)) {
        printf("[Sensor] INA238 initialized successfully\n");
        g_sensor_data.ina238_valid = true;
    } else {
        printf("[Sensor] WARNING: INA238 initialization failed\n");
        g_sensor_data.ina238_valid = false;
    }
    
    /* Initialize BME280 */
    printf("[Sensor] Initializing BME280 at address 0x%02X...\n", BME280_I2C_ADDR);
    if (bme280_init(&g_bme280, I2C_PORT, BME280_I2C_ADDR)) {
        printf("[Sensor] BME280 initialized successfully\n");
        g_sensor_data.bme280_valid = true;
    } else {
        printf("[Sensor] WARNING: BME280 initialization failed\n");
        g_sensor_data.bme280_valid = false;
    }
}

void sensor_task(void *params) {
    (void)params;
    
    TickType_t last_wake_time;
    ina238_data_t ina_data;
    bme280_data_t bme_data;
    
    printf("[Sensor] Task started\n");
    
    /* Initialize I2C and sensors */
    init_i2c();
    init_sensors();
    
    /* Set default poll interval */
    g_sensor_data.poll_interval_ms = DEFAULT_POLL_INTERVAL_MS;
    g_sensor_data.led_state = false;
    
    last_wake_time = xTaskGetTickCount();
    
    while (1) {
        uint32_t poll_interval;
        
        /* Get current poll interval */
        if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            poll_interval = g_sensor_data.poll_interval_ms;
            xSemaphoreGive(g_sensor_mutex);
        } else {
            poll_interval = DEFAULT_POLL_INTERVAL_MS;
        }
        
        /* Read INA238 */
        if (g_ina238.initialized) {
            if (ina238_read(&g_ina238, &ina_data)) {
                if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    g_sensor_data.voltage_v = ina_data.bus_voltage_v;
                    g_sensor_data.current_ma = ina_data.current_ma;
                    g_sensor_data.ina238_valid = true;
                    xSemaphoreGive(g_sensor_mutex);
                }
            } else {
                if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    g_sensor_data.ina238_valid = false;
                    xSemaphoreGive(g_sensor_mutex);
                }
            }
        }
        
        /* Read BME280 */
        if (g_bme280.initialized) {
            if (bme280_read(&g_bme280, &bme_data)) {
                if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    g_sensor_data.temperature_c = bme_data.temperature_c;
                    g_sensor_data.humidity_pct = bme_data.humidity_pct;
                    g_sensor_data.bme280_valid = true;
                    xSemaphoreGive(g_sensor_mutex);
                }
            } else {
                if (xSemaphoreTake(g_sensor_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    g_sensor_data.bme280_valid = false;
                    xSemaphoreGive(g_sensor_mutex);
                }
            }
        }
        
        /* Wait for next poll interval */
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(poll_interval));
    }
}

void sensor_task_init(void) {
    /* Create mutex for sensor data access */
    g_sensor_mutex = xSemaphoreCreateMutex();
    configASSERT(g_sensor_mutex != NULL);
    
    /* Create sensor task */
    BaseType_t result = xTaskCreate(
        sensor_task,
        "sensor",
        SENSOR_TASK_STACK_SIZE,
        NULL,
        SENSOR_TASK_PRIORITY,
        &g_sensor_task_handle
    );
    configASSERT(result == pdPASS);
    
    printf("[Sensor] Task created\n");
}
