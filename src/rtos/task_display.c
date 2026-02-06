#include "rtos_tasks.h"

#include <stdio.h>

#include "sensor_data.h"
#include "ssd1306.h"

#include "FreeRTOS.h"
#include "task.h"

typedef enum {
    SCREEN_LUMINOSITY,
    SCREEN_TEMPERATURE,
    SCREEN_COUNT
} display_screen_t;

void task_display(void *param) {
    const rtos_task_params_t *params = (const rtos_task_params_t *)param;
    const app_context_t *ctx = params ? params->ctx : NULL;

    if (!ctx || !ctx->display) {
        vTaskDelete(NULL);
    }

    display_screen_t current_screen = SCREEN_LUMINOSITY;
    uint32_t screen_timer = 0;
    const uint32_t SCREEN_DURATION_MS = 3000;

    while (true) {
        sensor_data_t data = sensor_data_get();

        if (current_screen == SCREEN_LUMINOSITY) {
            char lux_str[32];
            char intensity_str[32];
            char status_str[32];

            if (data.luminosity_valid) {
                snprintf(lux_str, sizeof(lux_str), "%.1f lux", data.luminosity_lux);

                led_intensity_t intensity = led_matrix_get_intensity_from_lux(data.luminosity_lux);
                switch (intensity) {
                    case LED_INTENSITY_OFF:
                        snprintf(intensity_str, sizeof(intensity_str), "Desligado");
                        break;
                    case LED_INTENSITY_LOW:
                        snprintf(intensity_str, sizeof(intensity_str), "Fraco");
                        break;
                    case LED_INTENSITY_MEDIUM:
                        snprintf(intensity_str, sizeof(intensity_str), "Medio");
                        break;
                    case LED_INTENSITY_HIGH:
                        snprintf(intensity_str, sizeof(intensity_str), "Forte");
                        break;
                }

                snprintf(status_str, sizeof(status_str), data.led_matrix_enabled ? "ON" : "OFF");
            } else {
                snprintf(lux_str, sizeof(lux_str), "Erro");
                snprintf(intensity_str, sizeof(intensity_str), "--");
                snprintf(status_str, sizeof(status_str), "--");
            }

            ssd1306_clear(ctx->display);
            ssd1306_draw_string(ctx->display, 0, 0, "==Luminosidade==");
            ssd1306_draw_string(ctx->display, 0, 16, "Luz:");
            ssd1306_draw_string(ctx->display, 35, 16, lux_str);
            ssd1306_draw_string(ctx->display, 0, 32, "LED:");
            ssd1306_draw_string(ctx->display, 35, 32, intensity_str);
            ssd1306_draw_string(ctx->display, 0, 48, "Status:");
            ssd1306_draw_string(ctx->display, 50, 48, status_str);
            ssd1306_show(ctx->display);
        } else if (current_screen == SCREEN_TEMPERATURE) {
            char temp_str[32];
            char humid_str[32];

            if (data.temp_humidity_valid) {
                snprintf(temp_str, sizeof(temp_str), "%.1f*C", data.temperature_c);
                snprintf(humid_str, sizeof(humid_str), "%.1f%%", data.humidity_percent);
            } else {
                snprintf(temp_str, sizeof(temp_str), "Erro");
                snprintf(humid_str, sizeof(humid_str), "Erro");
            }

            ssd1306_clear(ctx->display);
            ssd1306_draw_string(ctx->display, 0, 0, "=Temp & Umidade=");
            ssd1306_draw_string(ctx->display, 0, 14, "Temp:");
            ssd1306_draw_string(ctx->display, 50, 14, temp_str);
            ssd1306_draw_string(ctx->display, 0, 32, "Umid:");
            ssd1306_draw_string(ctx->display, 50, 32, humid_str);
            ssd1306_show(ctx->display);
        }

        screen_timer += 200;
        if (screen_timer >= SCREEN_DURATION_MS) {
            screen_timer = 0;
            current_screen = (current_screen + 1) % SCREEN_COUNT;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
