/*
 * Webserver Task - HTTP server with CGI/SSI support
 */

#ifndef WEBSERVER_TASK_H
#define WEBSERVER_TASK_H

#include <stdbool.h>

/**
 * @brief Create and start the webserver task
 */
void webserver_task_init(void);

/**
 * @brief Webserver task function (FreeRTOS task)
 * 
 * @param params Task parameters (unused)
 */
void webserver_task(void *params);

#endif /* WEBSERVER_TASK_H */
