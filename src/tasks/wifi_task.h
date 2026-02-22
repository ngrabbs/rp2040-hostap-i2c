/*
 * WiFi Task - Access Point mode management
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <stdbool.h>

/**
 * @brief Check if WiFi AP is ready
 * 
 * @return true if AP is initialized and ready
 */
bool wifi_task_is_ready(void);

/**
 * @brief Create and start the WiFi task
 */
void wifi_task_init(void);

/**
 * @brief WiFi task function (FreeRTOS task)
 * 
 * @param params Task parameters (unused)
 */
void wifi_task(void *params);

#endif /* WIFI_TASK_H */
