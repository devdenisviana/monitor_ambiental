#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "ssd1306.h"
#include "bh1750.h"
#include "led_matrix.h"

// I2C0 para o sensor BH1750 - Pinos GP0 (SDA) e GP1 (SCL)
#define I2C0_PORT i2c0
#define I2C0_SDA 0
#define I2C0_SCL 1

// I2C1 para o display OLED SSD1306 - Pinos GP14 (SDA) e GP15 (SCL)
#define I2C1_PORT i2c1
#define I2C1_SDA 14
#define I2C1_SCL 15

// GPIO para matriz de LEDs 5x5
#define LED_MATRIX_PIN 7

// Função para escanear dispositivos I2C
void i2c_scan(i2c_inst_t *i2c, const char *bus_name) {
    printf("\n=== Scanner I2C - %s ===\n", bus_name);
    printf("Escaneando enderecos 0x00 a 0x7F...\n");
    
    int count = 0;
    for (uint8_t addr = 0; addr < 0x80; addr++) {
        uint8_t data;
        int result = i2c_read_blocking(i2c, addr, &data, 1, false);
        
        if (result >= 0) {
            printf("Dispositivo encontrado no endereco 0x%02X\n", addr);
            count++;
        }
    }
    
    if (count == 0) {
        printf("Nenhum dispositivo encontrado!\n");
    } else {
        printf("Total: %d dispositivo(s) encontrado(s)\n", count);
    }
    printf("========================\n\n");
}

int main()
{
    stdio_init_all();
    
    // Aguarda 3 segundos para você abrir o monitor serial
    printf("\n\n\n");
    printf("========================================\n");
    printf("    MONITOR AMBIENTAL - INICIANDO\n");
    printf("========================================\n");
    sleep_ms(3000);

    printf("\n[INFO] Inicializando I2C0 (GP4/GP5)...\n");
    // Inicialização do I2C0 para o sensor BH1750
    i2c_init(I2C0_PORT, 400 * 1000); // 400 KHz
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);
    printf("[OK] I2C0 inicializado\n");

    printf("\n[INFO] Inicializando I2C1 (GP14/GP15)...\n");
    // Inicialização do I2C1 para o display OLED
    i2c_init(I2C1_PORT, 400 * 1000); // 400 KHz
    gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA);
    gpio_pull_up(I2C1_SCL);
    printf("[OK] I2C1 inicializado\n");

    // Scanner I2C - Detecta dispositivos conectados
    sleep_ms(500);
    i2c_scan(I2C0_PORT, "I2C0 (GP4/GP5)");
    i2c_scan(I2C1_PORT, "I2C1 (GP14/GP15)");

    printf("\n[INFO] Inicializando display OLED SSD1306...\n");
    // Inicialização do display OLED SSD1306 no I2C1
    ssd1306_t display;
    ssd1306_init(&display, I2C1_PORT, SSD1306_I2C_ADDR);
    printf("[OK] Display OLED inicializado\n");
    
    printf("\n[INFO] Inicializando sensor BH1750 no endereco 0x23...\n");
    // Inicialização do sensor BH1750 no I2C0
    bh1750_t light_sensor;
    bool bh1750_ok = bh1750_init(&light_sensor, I2C0_PORT, BH1750_ADDR_LOW);
    
    if (!bh1750_ok) {
        printf("[ERRO] Falha ao inicializar BH1750 no endereco 0x23!\n");
        printf("[INFO] Tentando endereco alternativo 0x5C...\n");
        bh1750_ok = bh1750_init(&light_sensor, I2C0_PORT, BH1750_ADDR_HIGH);
        if (bh1750_ok) {
            printf("[OK] BH1750 inicializado no endereco 0x5C\n");
        } else {
            printf("[ERRO] BH1750 nao encontrado em nenhum endereco!\n");
        }
    } else {
        printf("[OK] BH1750 inicializado no endereco 0x23\n");
    }
    
    printf("\n[INFO] Inicializando matriz de LEDs...\n");
    // Inicialização da matriz de LEDs no GPIO 7
    led_matrix_t led_matrix;
    led_matrix_init(&led_matrix, LED_MATRIX_PIN);
    
    // Tela inicial - Mostra o nome do programa
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 8, 28, "Monitor Ambiental");
    ssd1306_show(&display);
    
    sleep_ms(3000); // Aguarda 3 segundos mostrando o título
    
    printf("Monitor Ambiental iniciado!\n");

    // Loop principal - Exibe as leituras do sensor
    while (true) {
        float lux = 0;
        char lux_str[32];
        char intensity_str[32];
        
        // Lê o sensor de luminosidade
        if (bh1750_ok && bh1750_read_light(&light_sensor, &lux)) {
            snprintf(lux_str, sizeof(lux_str), "Luz: %.1f lux", lux);
            printf("%s\n", lux_str);
            
            // Determina e aplica intensidade dos LEDs baseado na luminosidade
            led_intensity_t intensity = led_matrix_get_intensity_from_lux(lux);
            led_matrix_set_intensity(&led_matrix, intensity);
            
            // Define texto da intensidade para exibir no display
            switch (intensity) {
                case LED_INTENSITY_OFF:
                    snprintf(intensity_str, sizeof(intensity_str), "LED: Desligado");
                    break;
                case LED_INTENSITY_LOW:
                    snprintf(intensity_str, sizeof(intensity_str), "LED: Fraco");
                    break;
                case LED_INTENSITY_MEDIUM:
                    snprintf(intensity_str, sizeof(intensity_str), "LED: Medio");
                    break;
                case LED_INTENSITY_HIGH:
                    snprintf(intensity_str, sizeof(intensity_str), "LED: Forte");
                    break;
            }
        } else {
            snprintf(lux_str, sizeof(lux_str), "Luz: Erro");
            snprintf(intensity_str, sizeof(intensity_str), "LED: --");
            printf("Erro ao ler BH1750\n");
        }
        
        // Atualiza o display com as leituras
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 15, "Luminosidade:");
        ssd1306_draw_string(&display, 0, 28, lux_str);
        ssd1306_draw_string(&display, 0, 45, intensity_str);
        ssd1306_show(&display);
        
        sleep_ms(1000);
    }
}
