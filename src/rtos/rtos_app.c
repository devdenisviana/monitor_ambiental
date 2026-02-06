#include "rtos_app.h"
#include "rtos_tasks.h"
#include "sensor_data.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static rtos_task_params_t g_task_params;

void rtos_start(const app_context_t *ctx) {
    if (!ctx) {
        printf("[RTOS] Contexto invalido\n");
        return;
    }

    g_task_params.ctx = ctx;
    g_task_params.sensor_mutex = xSemaphoreCreateMutex();

    if (g_task_params.sensor_mutex) {
        sensor_data_set_mutex(g_task_params.sensor_mutex);
    } else {
        printf("[RTOS] ERRO: falha ao criar mutex de sensores\n");
    }

    xTaskCreate(task_sensors, "sensors", 1024, &g_task_params, 2, NULL);
    xTaskCreate(task_display, "display", 1024, &g_task_params, 1, NULL);
    xTaskCreate(task_uart, "uart", 1024, &g_task_params, 1, NULL);
    xTaskCreate(task_web, "web", 1024, &g_task_params, 1, NULL);

    vTaskStartScheduler();

    // Se chegar aqui, houve erro ao iniciar o scheduler
    printf("[RTOS] Scheduler nao iniciou\n");
    while (true) {
    }
}
