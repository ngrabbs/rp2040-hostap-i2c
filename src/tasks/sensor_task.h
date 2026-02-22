/*
 * Sensor Task - I2C sensor polling for INA238 and BME280
 */

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* Sensor data structure shared with webserver */
typedef struct {
    /* INA238 data */
    float voltage_v;
    float current_ma;
    
    /* BME280 data */
    float temperature_c;
    float humidity_pct;
    
    /* Status flags */
    bool ina238_valid;
    bool bme280_valid;
    
    /* Polling configuration */
    uint32_t poll_interval_ms;
    
    /* LED/payload state */
    bool led_state;
} sensor_data_t;

/**
 * @brief Get pointer to the shared sensor data
 * 
 * @return Pointer to sensor data structure
 */
sensor_data_t* sensor_task_get_data(void);

/**
 * @brief Get the mutex protecting sensor data
 * 
 * @return Mutex handle
 */
SemaphoreHandle_t sensor_task_get_mutex(void);

/**
 * @brief Set the polling interval
 * 
 * @param interval_ms New polling interval in milliseconds
 */
void sensor_task_set_poll_interval(uint32_t interval_ms);

/**
 * @brief Set the LED/payload state
 * 
 * @param state true = ON, false = OFF
 */
void sensor_task_set_led(bool state);

/**
 * @brief Get the LED/payload state
 * 
 * @return true = ON, false = OFF
 */
bool sensor_task_get_led(void);

/**
 * @brief Create and start the sensor task
 */
void sensor_task_init(void);

/**
 * @brief Sensor task function (FreeRTOS task)
 * 
 * @param params Task parameters (unused)
 */
void sensor_task(void *params);

#endif /* SENSOR_TASK_H */
