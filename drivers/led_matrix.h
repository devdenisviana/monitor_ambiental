#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define LED_MATRIX_SIZE 25  // 5x5 matriz

// Definição dos níveis de intensidade
typedef enum {
    LED_INTENSITY_OFF = 0,
    LED_INTENSITY_LOW = 1,     // Fraco
    LED_INTENSITY_MEDIUM = 2,  // Médio
    LED_INTENSITY_HIGH = 3     // Forte
} led_intensity_t;

// Estrutura da matriz de LEDs
typedef struct {
    uint gpio_pin;
    PIO pio;
    uint sm;
    led_intensity_t current_intensity;
} led_matrix_t;

// Funções públicas
void led_matrix_init(led_matrix_t *matrix, uint gpio_pin);
void led_matrix_set_intensity(led_matrix_t *matrix, led_intensity_t intensity);
void led_matrix_clear(led_matrix_t *matrix);
led_intensity_t led_matrix_get_intensity_from_lux(float lux);

#endif // LED_MATRIX_H
