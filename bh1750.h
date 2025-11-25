#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Endereços I2C possíveis do BH1750
#define BH1750_ADDR_LOW  0x23  // Quando ADDR pin está em LOW ou flutuante
#define BH1750_ADDR_HIGH 0x5C  // Quando ADDR pin está em HIGH

// Comandos do BH1750
#define BH1750_POWER_DOWN           0x00
#define BH1750_POWER_ON             0x01
#define BH1750_RESET                0x07
#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10  // 1 lx resolução, 120ms
#define BH1750_CONTINUOUS_HIGH_RES_MODE2 0x11  // 0.5 lx resolução, 120ms
#define BH1750_CONTINUOUS_LOW_RES_MODE   0x13  // 4 lx resolução, 16ms
#define BH1750_ONE_TIME_HIGH_RES_MODE    0x20  // 1 lx resolução, 120ms (modo único)
#define BH1750_ONE_TIME_HIGH_RES_MODE2   0x21  // 0.5 lx resolução, 120ms (modo único)
#define BH1750_ONE_TIME_LOW_RES_MODE     0x23  // 4 lx resolução, 16ms (modo único)

// Estrutura do sensor
typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t address;
} bh1750_t;

// Funções públicas
bool bh1750_init(bh1750_t *sensor, i2c_inst_t *i2c_port, uint8_t address);
bool bh1750_read_light(bh1750_t *sensor, float *lux);
void bh1750_power_down(bh1750_t *sensor);
void bh1750_power_on(bh1750_t *sensor);

#endif // BH1750_H
