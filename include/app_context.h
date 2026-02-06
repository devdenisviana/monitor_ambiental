#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <stdbool.h>
#include "ssd1306.h"
#include "bh1750.h"
#include "aht10.h"
#include "led_matrix.h"

typedef struct {
    ssd1306_t *display;
    bh1750_t *light_sensor;
    aht10_t *temp_sensor;
    led_matrix_t *led_matrix;
    volatile bool *led_matrix_enabled;
    bool *bh1750_ok;
    bool *aht10_ok;
} app_context_t;

#endif // APP_CONTEXT_H
