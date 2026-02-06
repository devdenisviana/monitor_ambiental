#include "aht10.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Envia comandos para o AHT10
static void enviar_comandos(aht10_t *sensor, const uint8_t *comandos, size_t tamanho) {
    i2c_write_blocking(sensor->i2c_port, sensor->address, comandos, tamanho, false);
}

// Reset por software
void aht10_soft_reset(aht10_t *sensor) {
    uint8_t cmd[3] = {
        AHT10_CMD_SOFT_RESET,
        AHT10_CMD_SOFT_RESET_ARG,
        AHT10_CMD_SOFT_RESET_ARG2
    };
    enviar_comandos(sensor, cmd, sizeof(cmd));
    sleep_ms(20);
}

// Calibra o sensor
static void calibrar_aht10(aht10_t *sensor) {
    uint8_t cmd[3] = {
        AHT10_CMD_CALIBRATION,
        AHT10_CMD_CALIBRATION_ARG,
        AHT10_CMD_CALIBRATION_ARG2
    };
    enviar_comandos(sensor, cmd, sizeof(cmd));
    sleep_ms(10);
}

// Envia comando de medição
static void enviar_comando_medicao(aht10_t *sensor) {
    uint8_t cmd[3] = {
        AHT10_CMD_MEASURE,
        AHT10_CMD_MEASURE_ARG,
        AHT10_CMD_MEASURE_ARG2
    };
    enviar_comandos(sensor, cmd, sizeof(cmd));
}

// Lê 6 bytes de dados do sensor
static void ler_dados_aht10(aht10_t *sensor, uint8_t *buf) {
    i2c_read_blocking(sensor->i2c_port, sensor->address, buf, 6, false);
}

// Processa os dados brutos do sensor
static void processar_dados_aht10(aht10_t *sensor, uint8_t *buf) {
    sensor->amostragem_umidade = ((uint32_t)buf[1] << 12) | 
                                  ((uint32_t)buf[2] << 4) | 
                                  ((buf[3] >> 4) & 0x0F);
    
    sensor->amostragem_temperatura = (((uint32_t)(buf[3] & 0x0F)) << 16) | 
                                      ((uint32_t)buf[4] << 8) | 
                                      buf[5];
}

// Obtém temperatura em Celsius
static float obter_temperatura_celsius(aht10_t *sensor) {
    return ((sensor->amostragem_temperatura * 200.0f) / (1 << 20)) - 50.0f;
}

// Obtém umidade relativa em %
static float obter_umidade_relativa(aht10_t *sensor) {
    return (sensor->amostragem_umidade * 100.0f) / (1 << 20);
}

// Inicializa o sensor AHT10
void aht10_init(aht10_t *sensor, i2c_inst_t *i2c_port, uint8_t address) {
    sensor->i2c_port = i2c_port;
    sensor->address = address;
    sensor->leitura_disponivel = false;
    sensor->amostragem_temperatura = 0;
    sensor->amostragem_umidade = 0;
    
    // Reset e calibração (sequência importante!)
    aht10_soft_reset(sensor);
    sleep_ms(20);
    calibrar_aht10(sensor);
    sleep_ms(10);
}

// Lê temperatura e umidade do sensor
bool aht10_read_temperature_humidity(aht10_t *sensor, float *temperature, float *humidity) {
    sensor->leitura_disponivel = false;
    
    // Envia comando de medição
    enviar_comando_medicao(sensor);
    
    // Aguarda 80ms para medição completar
    sleep_ms(80);
    
    // Lê dados
    uint8_t buf[6];
    ler_dados_aht10(sensor, buf);
    
    // Processa dados brutos
    processar_dados_aht10(sensor, buf);
    
    // Converte para valores reais
    *temperature = obter_temperatura_celsius(sensor);
    *humidity = obter_umidade_relativa(sensor);
    
    sensor->leitura_disponivel = true;
    
    return sensor->leitura_disponivel;
}
