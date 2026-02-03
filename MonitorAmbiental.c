#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "ssd1306.h"
#include "bh1750.h"
#include "aht10.h"
#include "led_matrix.h"
#include "sensor_data.h"
#include "wifi_manager.h"
#include "wifi_config.h"
#include "web_server.h"

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

// Botões da BitDogLab
#define BTN_A 5
#define BTN_B 6

// Variável global para controlar estado da matriz de LEDs
volatile bool led_matrix_enabled = true;

// Variável global para controlar qual tela exibir
typedef enum {
    SCREEN_LUMINOSITY,
    SCREEN_TEMPERATURE,
    SCREEN_COUNT  // Total de telas
} display_screen_t;

volatile display_screen_t current_screen = SCREEN_LUMINOSITY;
volatile uint32_t screen_timer = 0;
const uint32_t SCREEN_DURATION_MS = 3000;  // Cada tela fica 3 segundos

// Função para escanear dispositivos I2C
void i2c_scan(i2c_inst_t *i2c, const char *bus_name) {
    printf("\n=== Scanner I2C - %s ===\n", bus_name);
    printf("Escaneando enderecos 0x00 a 0x7F...\n");
    fflush(stdout);
    
    int count = 0;
    for (uint8_t addr = 0; addr < 0x80; addr++) {
        uint8_t data;
        int result = i2c_read_blocking(i2c, addr, &data, 1, false);
        
        if (result >= 0) {
            printf("Dispositivo encontrado no endereco 0x%02X\n", addr);
            fflush(stdout);
            count++;
        }
    }
    
    if (count == 0) {
        printf("Nenhum dispositivo encontrado!\n");
    } else {
        printf("Total: %d dispositivo(s) encontrado(s)\n", count);
    }
    printf("========================\n\n");
    fflush(stdout);
}

int main()
{
    stdio_init_all();
    
    // Aguarda um pouco para estabilizar USB
    sleep_ms(1000);
    
    printf("\n\n\n");
    printf("========================================\n");
    printf("    MONITOR AMBIENTAL - INICIANDO\n");
    printf("========================================\n");
    fflush(stdout);

    // ===== INICIALIZA I2C E DISPLAY PRIMEIRO (para diagnóstico) =====
    printf("\n[INFO] Inicializando I2C1 para display...\n");
    fflush(stdout);
    
    i2c_init(I2C1_PORT, 400 * 1000);
    gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA);
    gpio_pull_up(I2C1_SCL);
    
    ssd1306_t display;
    ssd1306_init(&display, I2C1_PORT, SSD1306_I2C_ADDR);
    
    // Mostra mensagem no display ANTES de inicializar WiFi
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, "Monitor Ambiental");
    ssd1306_draw_string(&display, 0, 16, "Iniciando...");
    ssd1306_draw_string(&display, 0, 32, "WiFi: Aguarde");
    ssd1306_show(&display);
    
    printf("[OK] Display inicializado\n");
    fflush(stdout);
    
    // ===== AGORA INICIALIZA O WIFI =====
    printf("\n[INFO] Inicializando chip CYW43 (WiFi)...\n");
    fflush(stdout);
    
    // Atualiza display
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, "Monitor Ambiental");
    ssd1306_draw_string(&display, 0, 16, "Init CYW43...");
    ssd1306_show(&display);
    
    bool wifi_chip_ok = wifi_manager_init();
    if (wifi_chip_ok) {
        printf("[OK] Chip CYW43 inicializado\n");
        ssd1306_draw_string(&display, 0, 32, "CYW43: OK");
    } else {
        printf("[ERRO] Falha ao inicializar chip CYW43!\n");
        ssd1306_draw_string(&display, 0, 32, "CYW43: ERRO!");
    }
    ssd1306_show(&display);
    fflush(stdout);
    
    sleep_ms(2000);

    // Inicialização dos botões
    printf("\n[INFO] Inicializando botoes...\n");
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);
    printf("[OK] Botoes A e B inicializados\n");
    fflush(stdout);

    printf("\n[INFO] Inicializando I2C0 (GP0/GP1)...\n");
    // Inicialização do I2C0 para sensores BH1750 e AHT10
    // IMPORTANTE: AHT10 funciona melhor com 100 kHz
    i2c_init(I2C0_PORT, 100 * 1000); // 100 KHz (compatível com AHT10)
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);
    printf("[OK] I2C0 inicializado em 100 kHz\n");
    fflush(stdout);

    // I2C1 já foi inicializado no início para o display
    printf("[OK] I2C1 ja inicializado (display)\n");
    fflush(stdout);

    // Scanner I2C - Detecta dispositivos conectados
    sleep_ms(500);
    i2c_scan(I2C0_PORT, "I2C0 (GP0/GP1)");
    i2c_scan(I2C1_PORT, "I2C1 (GP14/GP15)");

    // Display já foi inicializado no início - não precisa reinicializar
    printf("[OK] Display OLED ja inicializado\n");
    fflush(stdout);
    
    printf("\n[INFO] Inicializando sensor BH1750 no endereco 0x23...\n");
    // Inicialização do sensor BH1750 no I2C0
    bh1750_t light_sensor;
    bool bh1750_ok = bh1750_init(&light_sensor, I2C0_PORT, BH1750_ADDR_LOW);
    
    if (!bh1750_ok) {
        printf("[ERRO] Falha ao inicializar BH1750 no endereco 0x23!\n");
        printf("[INFO] Tentando endereco alternativo 0x5C...\n");
        fflush(stdout);
        bh1750_ok = bh1750_init(&light_sensor, I2C0_PORT, BH1750_ADDR_HIGH);
        if (bh1750_ok) {
            printf("[OK] BH1750 inicializado no endereco 0x5C\n");
        } else {
            printf("[ERRO] BH1750 nao encontrado em nenhum endereco!\n");
        }
    } else {
        printf("[OK] BH1750 inicializado no endereco 0x23\n");
    }
    fflush(stdout);
    
    printf("\n[INFO] Inicializando sensor AHT10 no endereco 0x38...\n");
    fflush(stdout);
    // Inicialização do sensor AHT10 no I2C0
    aht10_t temp_sensor;
    aht10_init(&temp_sensor, I2C0_PORT, AHT10_I2C_ADDR);
    printf("[OK] AHT10 inicializado no endereco 0x38\n");
    fflush(stdout);
    
    // Marca AHT10 como OK (sempre, pois init não retorna bool)
    bool aht10_ok = true;
    
    printf("\n[INFO] Inicializando matriz de LEDs...\n");
    // Inicialização da matriz de LEDs no GPIO 7
    led_matrix_t led_matrix;
    led_matrix_init(&led_matrix, LED_MATRIX_PIN);
    
    // Inicializa estrutura de dados compartilhada
    printf("\n[INFO] Inicializando estrutura de dados...\n");
    sensor_data_init();
    printf("[OK] Estrutura de dados inicializada\n");
    fflush(stdout);
    
    // ========== CONEXÃO WIFI ==========
    // (O chip CYW43 já foi inicializado no início do main)
    bool wifi_ok = false;
    if (wifi_chip_ok) {
        printf("\n[INFO] Conectando ao WiFi...\n");
        printf("[INFO] SSID: %s\n", WIFI_SSID);
        printf("[INFO] Timeout: 30 segundos\n");
        fflush(stdout);
        
        // Mostra no display que está conectando
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, "WiFi:");
        ssd1306_draw_string(&display, 0, 16, "Conectando...");
        ssd1306_draw_string(&display, 0, 32, WIFI_SSID);
        ssd1306_draw_string(&display, 0, 48, "Aguarde 30s...");
        ssd1306_show(&display);
        
        // Tenta conectar ao WiFi com timeout de 30 segundos (usar o padrão)
        wifi_ok = wifi_manager_connect(WIFI_SSID, WIFI_PASSWORD, 0);
        
        if (wifi_ok) {
            printf("[OK] WiFi conectado com sucesso!\n");
            printf("[OK] IP: %s\n", wifi_manager_get_ip());
            fflush(stdout);
            
            // Inicializa servidor web
            if (web_server_init(WEB_SERVER_PORT)) {
                printf("[OK] Servidor web disponivel em: http://%s\n", wifi_manager_get_ip());
            } else {
                printf("[ERRO] Falha ao iniciar servidor web\n");
            }
            fflush(stdout);
            
            // Mostra IP no display
            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, "WiFi Conectado!");
            ssd1306_draw_string(&display, 0, 16, "IP:");
            ssd1306_draw_string(&display, 0, 32, wifi_manager_get_ip());
            ssd1306_draw_string(&display, 0, 48, "http://");
            ssd1306_show(&display);
            sleep_ms(5000);  // Aumentado para 5 segundos para ver o IP
        } else {
            printf("[ERRO] Falha ao conectar WiFi!\n");
            printf("[ERRO] Verifique SSID e senha\n");
            fflush(stdout);
            
            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, "WiFi ERRO!");
            ssd1306_draw_string(&display, 0, 16, "Continuando");
            ssd1306_draw_string(&display, 0, 32, "sem WiFi...");
            ssd1306_show(&display);
            sleep_ms(3000);
        }
    } else {
        printf("[ERRO] Chip WiFi nao disponivel!\n");
        fflush(stdout);
    }
    
    // Tela inicial - Mostra o nome do programa
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 8, 28, "Monitor Ambiental");
    ssd1306_show(&display);
    
    sleep_ms(3000); // Aguarda 3 segundos mostrando o título
    
    printf("Monitor Ambiental iniciado!\n");
    fflush(stdout);

    // Loop principal - Exibe as leituras dos sensores com rotação de telas
    while (true) {
        // Verifica botão A (desativa matriz de LEDs)
        if (gpio_get(BTN_A) == 0) {  // Botão pressionado (pull-up, então 0 = pressionado)
            if (led_matrix_enabled) {
                led_matrix_enabled = false;
                led_matrix_clear(&led_matrix);
                printf("[BTN A] Matriz de LEDs DESATIVADA\n");
                fflush(stdout);
                sleep_ms(300);  // Debounce
            }
        }
        
        // Verifica botão B (ativa matriz de LEDs)
        if (gpio_get(BTN_B) == 0) {  // Botão pressionado
            if (!led_matrix_enabled) {
                led_matrix_enabled = true;
                printf("[BTN B] Matriz de LEDs ATIVADA\n");
                fflush(stdout);
                sleep_ms(300);  // Debounce
            }
        }
        
        // Atualiza timer de tela
        screen_timer += 200;
        if (screen_timer >= SCREEN_DURATION_MS) {
            screen_timer = 0;
            current_screen = (current_screen + 1) % SCREEN_COUNT;
        }
        
        // ========== TELA 1: LUMINOSIDADE ==========
        if (current_screen == SCREEN_LUMINOSITY) {
            float lux = 0;
            char lux_str[32];
            char intensity_str[32];
            char status_str[32];
            
            // Lê o sensor de luminosidade
            if (bh1750_ok && bh1750_read_light(&light_sensor, &lux)) {
                snprintf(lux_str, sizeof(lux_str), "%.1f lux", lux);
                printf("Luminosidade: %s\n", lux_str);
                fflush(stdout);
                
                // Atualiza dados globais para o servidor web
                sensor_data_set_luminosity(lux, true);
                
                // Determina intensidade baseado na luminosidade
                led_intensity_t intensity = led_matrix_get_intensity_from_lux(lux);
                
                // Aplica intensidade APENAS se a matriz estiver ativada
                if (led_matrix_enabled) {
                    led_matrix_set_intensity(&led_matrix, intensity);
                    snprintf(status_str, sizeof(status_str), "ON");
                } else {
                    snprintf(status_str, sizeof(status_str), "OFF");
                }
                
                // Define texto da intensidade para exibir no display
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
            } else {
                snprintf(lux_str, sizeof(lux_str), "Erro");
                snprintf(intensity_str, sizeof(intensity_str), "--");
                snprintf(status_str, sizeof(status_str), "--");
                printf("Erro ao ler BH1750\n");
                fflush(stdout);
            }
            
            // Exibe tela de luminosidade
            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, "==Luminosidade==");
            ssd1306_draw_string(&display, 0, 16, "Luz:");
            ssd1306_draw_string(&display, 35, 16, lux_str);
            ssd1306_draw_string(&display, 0, 32, "LED:");
            ssd1306_draw_string(&display, 35, 32, intensity_str);
            ssd1306_draw_string(&display, 0, 48, "Status:");
            ssd1306_draw_string(&display, 50, 48, status_str);
            ssd1306_show(&display);
        }
        
        // ========== TELA 2: TEMPERATURA E UMIDADE ==========
        else if (current_screen == SCREEN_TEMPERATURE) {
            float temperature = 0.0f;
            float humidity = 0.0f;
            char temp_str[32];
            char humid_str[32];
            
            // Lê o sensor de temperatura e umidade
            if (aht10_ok && aht10_read_temperature_humidity(&temp_sensor, &temperature, &humidity)) {
                snprintf(temp_str, sizeof(temp_str), "%.1f*C", temperature);
                snprintf(humid_str, sizeof(humid_str), "%.1f%%", humidity);
                printf("Temp: %s, Umid: %s\n", temp_str, humid_str);
                fflush(stdout);
                
                // Atualiza dados globais para o servidor web
                sensor_data_set_temp_humidity(temperature, humidity, true);
            } else {
                snprintf(temp_str, sizeof(temp_str), "Erro");
                snprintf(humid_str, sizeof(humid_str), "Erro");
                printf("Erro ao ler AHT10\n");
                fflush(stdout);
            }
            
            // Exibe tela de temperatura e umidade
            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, "=Temp & Umidade=");
            ssd1306_draw_string(&display, 0, 14, "Temp:");
            ssd1306_draw_string(&display, 50, 14, temp_str);
            ssd1306_draw_string(&display, 0, 32, "Umid:");
            ssd1306_draw_string(&display, 50, 32, humid_str);
            ssd1306_show(&display);
        }
        
        sleep_ms(200);  // Atualiza a cada 200ms
        
        // Polling do WiFi (mantém a conexão ativa)
        wifi_manager_poll();
    }

    return 0;
}
