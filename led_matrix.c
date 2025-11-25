#include "led_matrix.h"
#include "ws2812.pio.h"
#include <stdio.h>
#include <string.h>

static inline void put_pixel(led_matrix_t *matrix, uint32_t pixel_grb) {
    pio_sm_put_blocking(matrix->pio, matrix->sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b);
}

// Inicializa a matriz de LEDs WS2812 usando PIO
void led_matrix_init(led_matrix_t *matrix, uint gpio_pin) {
    matrix->gpio_pin = gpio_pin;
    matrix->current_intensity = LED_INTENSITY_OFF;
    matrix->pio = pio0;
    
    // Carrega o programa WS2812 no PIO
    uint offset = pio_add_program(matrix->pio, &ws2812_program);
    matrix->sm = pio_claim_unused_sm(matrix->pio, true);
    ws2812_program_init(matrix->pio, matrix->sm, offset, gpio_pin, 800000, false);
    
    // Limpa todos os LEDs
    for (int i = 0; i < LED_MATRIX_SIZE; i++) {
        put_pixel(matrix, 0);
    }
    
    printf("[OK] Matriz de LEDs WS2812 inicializada no GPIO %d\n", gpio_pin);
}

// Define a intensidade da matriz acendendo os LEDs com cor branca
void led_matrix_set_intensity(led_matrix_t *matrix, led_intensity_t intensity) {
    if (intensity == matrix->current_intensity) {
        return;
    }
    
    matrix->current_intensity = intensity;
    
    uint8_t brightness;
    
    switch (intensity) {
        case LED_INTENSITY_OFF:
            brightness = 0;
            printf("[LED] Desligado\n");
            break;
        case LED_INTENSITY_LOW:
            brightness = 3;  // Fraco (~1%)
            printf("[LED] Intensidade FRACA\n");
            break;
        case LED_INTENSITY_MEDIUM:
            brightness = 8;  // Médio (~3%)
            printf("[LED] Intensidade MEDIA\n");
            break;
        case LED_INTENSITY_HIGH:
            brightness = 25;  // Forte (~10%)
            printf("[LED] Intensidade FORTE\n");
            break;
        default:
            brightness = 0;
            break;
    }
    
    // Acende todos os LEDs da matriz com a cor branca na intensidade desejada
    for (int i = 0; i < LED_MATRIX_SIZE; i++) {
        put_pixel(matrix, urgb_u32(brightness, brightness, brightness));
    }
}

// Limpa/desliga todos os LEDs
void led_matrix_clear(led_matrix_t *matrix) {
    led_matrix_set_intensity(matrix, LED_INTENSITY_OFF);
}

// Converte valor de luminosidade (lux) para nível de intensidade dos LEDs
// Lógica inversa: quanto mais luz ambiente, MENOS intensidade nos LEDs
led_intensity_t led_matrix_get_intensity_from_lux(float lux) {
    // Thresholds de luminosidade
    // Ambientes típicos:
    // - Escuro (noite): 0-10 lux
    // - Ambiente interno com pouca luz: 10-100 lux
    // - Ambiente interno bem iluminado: 100-500 lux
    // - Luz solar indireta: 500-10000 lux
    // - Luz solar direta: 10000+ lux
    
    if (lux < 50) {
        // Muito escuro - LEDs no máximo
        return LED_INTENSITY_HIGH;
    } else if (lux < 200) {
        // Pouca luz - LEDs médio
        return LED_INTENSITY_MEDIUM;
    } else if (lux < 500) {
        // Luz moderada - LEDs fraco
        return LED_INTENSITY_LOW;
    } else {
        // Muita luz - LEDs desligados
        return LED_INTENSITY_OFF;
    }
}
