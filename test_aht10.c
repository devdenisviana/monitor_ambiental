#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "aht10.h"

#define I2C0_PORT i2c0
#define I2C0_SDA 0
#define I2C0_SCL 1

int main()
{
    stdio_init_all();
    
    printf("\n\n========================================\n");
    printf("    TESTE AHT10 - DIAGNOSTICO\n");
    printf("========================================\n");
    sleep_ms(2000);
    
    printf("\n[1] Inicializando I2C0...\n");
    fflush(stdout);
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);
    printf("[OK] I2C0 inicializado\n");
    fflush(stdout);
    
    sleep_ms(500);
    
    printf("\n[2] Scanner I2C0 (0x00-0x7F)...\n");
    fflush(stdout);
    
    int count = 0;
    for (uint8_t addr = 0; addr < 0x80; addr++) {
        uint8_t data = 0;
        int result = i2c_read_blocking(I2C0_PORT, addr, &data, 1, false);
        
        if (result >= 0) {
            printf("    >> Encontrado: 0x%02X\n", addr);
            fflush(stdout);
            count++;
        }
    }
    
    printf("Total: %d dispositivo(s)\n", count);
    fflush(stdout);
    
    sleep_ms(1000);
    
    printf("\n[3] Tentando inicializar AHT10 em 0x38...\n");
    fflush(stdout);
    
    aht10_t sensor;
    bool aht10_ok = aht10_init(&sensor, I2C0_PORT, AHT10_I2C_ADDR);
    
    if (aht10_ok) {
        printf("[OK] AHT10 inicializado com sucesso!\n");
        fflush(stdout);
        
        printf("\n[4] Tentando ler temperatura e umidade...\n");
        fflush(stdout);
        
        for (int i = 0; i < 5; i++) {
            float temp = 0, humid = 0;
            if (aht10_read_temperature_humidity(&sensor, &temp, &humid)) {
                printf("    Leitura %d: T=%.1f°C, H=%.1f%%\n", i+1, temp, humid);
            } else {
                printf("    Leitura %d: ERRO\n", i+1);
            }
            fflush(stdout);
            sleep_ms(1000);
        }
    } else {
        printf("[ERRO] Falha ao inicializar AHT10\n");
        fflush(stdout);
    }
    
    printf("\n[5] Teste finalizado\n");
    fflush(stdout);
    
    while(1) {
        sleep_ms(1000);
    }
    
    return 0;
}
