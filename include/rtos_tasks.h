#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#include "app_context.h"

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    const app_context_t *ctx;
    SemaphoreHandle_t sensor_mutex;
} rtos_task_params_t;

void task_sensors(void *param);
void task_display(void *param);
void task_uart(void *param);
void task_web(void *param);

#endif // RTOS_TASKS_H
