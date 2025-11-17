#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// I2C defines para o display OLED SSD1306 da BitDogLab
// Pinos conforme documentação: GP14 (SDA) e GP15 (SCL)
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15



int main()
{
    stdio_init_all();

    // Inicialização do I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000); // 400 KHz
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED SSD1306
    ssd1306_t display;
    ssd1306_init(&display, I2C_PORT, SSD1306_I2C_ADDR);
    
    // Limpa o display
    ssd1306_clear(&display);
    
    // Desenha o título "Monitor Ambiental" centralizado
    // Display tem 128 pixels de largura, cada char tem 6 pixels
    // "Monitor Ambiental" tem 17 caracteres = 102 pixels
    // Centralizado: (128 - 102) / 2 = 13 pixels
    ssd1306_draw_string(&display, 8, 28, "Monitor Ambiental");
    
    // Atualiza o display
    ssd1306_show(&display);
    
    printf("Monitor Ambiental iniciado!\n");

    while (true) {
        sleep_ms(1000);
    }
}
