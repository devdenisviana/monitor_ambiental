#include "rtos_tasks.h"

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensor_data.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"

// Botões da BitDogLab
#define BTN_A 5
#define BTN_B 6

#define BTN_EVENT_A (1u << 0)
#define BTN_EVENT_B (1u << 1)

static volatile uint32_t s_button_events = 0;
static TaskHandle_t s_button_task = NULL;

static void buttons_irq_handler(uint gpio, uint32_t events) {
    if ((events & GPIO_IRQ_EDGE_FALL) == 0) {
        return;
    }

    if (gpio == BTN_A) {
        s_button_events |= BTN_EVENT_A;
    } else if (gpio == BTN_B) {
        s_button_events |= BTN_EVENT_B;
    } else {
        return;
    }

    if (s_button_task) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(s_button_task, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void task_sensors(void *param) {
    const rtos_task_params_t *params = (const rtos_task_params_t *)param;
    const app_context_t *ctx = params ? params->ctx : NULL;

    if (!ctx || !ctx->light_sensor || !ctx->temp_sensor || !ctx->led_matrix || !ctx->led_matrix_enabled) {
        vTaskDelete(NULL);
    }

    s_button_task = xTaskGetCurrentTaskHandle();

    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &buttons_irq_handler);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);

    uint32_t last_btn_a_ms = 0;
    uint32_t last_btn_b_ms = 0;

    while (true) {
        (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(200));

        uint32_t events;
        taskENTER_CRITICAL();
        events = s_button_events;
        s_button_events = 0;
        taskEXIT_CRITICAL();

        if (events != 0) {
            uint32_t now_ms = to_ms_since_boot(get_absolute_time());

            if ((events & BTN_EVENT_A) != 0) {
                if ((now_ms - last_btn_a_ms) > 200) {
                    last_btn_a_ms = now_ms;
                    if (*ctx->led_matrix_enabled) {
                        *ctx->led_matrix_enabled = false;
                        led_matrix_clear(ctx->led_matrix);
                        sensor_data_set_led_state(false, LED_INTENSITY_OFF);
                    }
                }
            }

            if ((events & BTN_EVENT_B) != 0) {
                if ((now_ms - last_btn_b_ms) > 200) {
                    last_btn_b_ms = now_ms;
                    if (!*ctx->led_matrix_enabled) {
                        *ctx->led_matrix_enabled = true;
                        sensor_data_set_led_state(true, LED_INTENSITY_LOW);
                    }
                }
            }
        }

        float lux = 0.0f;
        float temperature = 0.0f;
        float humidity = 0.0f;

        if (ctx->bh1750_ok && *ctx->bh1750_ok && bh1750_read_light(ctx->light_sensor, &lux)) {
            sensor_data_set_luminosity(lux, true);

            led_intensity_t intensity = led_matrix_get_intensity_from_lux(lux);
            if (*ctx->led_matrix_enabled) {
                led_matrix_set_intensity(ctx->led_matrix, intensity);
                sensor_data_set_led_state(true, intensity);
            } else {
                led_matrix_clear(ctx->led_matrix);
                sensor_data_set_led_state(false, LED_INTENSITY_OFF);
            }
        } else {
            sensor_data_set_luminosity(0.0f, false);
        }

        if (ctx->aht10_ok && *ctx->aht10_ok && aht10_read_temperature_humidity(ctx->temp_sensor, &temperature, &humidity)) {
            sensor_data_set_temp_humidity(temperature, humidity, true);
        } else {
            sensor_data_set_temp_humidity(0.0f, 0.0f, false);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
