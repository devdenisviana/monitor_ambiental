#ifndef AHT10_H
#define AHT10_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Endereço I2C do AHT10
#define AHT10_I2C_ADDR  0x38

// Comandos do AHT10
#define AHT10_CMD_CALIBRATION       0xE1
#define AHT10_CMD_CALIBRATION_ARG   0x08
#define AHT10_CMD_CALIBRATION_ARG2  0x00

#define AHT10_CMD_MEASURE           0xAC
#define AHT10_CMD_MEASURE_ARG       0x33
#define AHT10_CMD_MEASURE_ARG2      0x00

#define AHT10_CMD_SOFT_RESET        0xBA
#define AHT10_CMD_SOFT_RESET_ARG    0x00
#define AHT10_CMD_SOFT_RESET_ARG2   0x00

// Estrutura do sensor
typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t address;
    bool leitura_disponivel;
    uint32_t amostragem_temperatura;
    uint32_t amostragem_umidade;
} aht10_t;

// Funções públicas
void aht10_init(aht10_t *sensor, i2c_inst_t *i2c_port, uint8_t address);
bool aht10_read_temperature_humidity(aht10_t *sensor, float *temperature, float *humidity);
void aht10_soft_reset(aht10_t *sensor);

#endif // AHT10_H
