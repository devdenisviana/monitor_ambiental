#include "rtos_tasks.h"

#include "wifi_manager.h"
#include "web_server.h"

#include "FreeRTOS.h"
#include "task.h"

void task_web(void *param) {
    (void)param;

    while (true) {
        wifi_manager_poll();
        web_server_poll();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
