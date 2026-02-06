#include "bh1750.h"
#include "pico/stdlib.h"

// Envia um comando para o BH1750
static bool bh1750_send_cmd(bh1750_t *sensor, uint8_t cmd) {
    int result = i2c_write_blocking(sensor->i2c_port, sensor->address, &cmd, 1, false);
    return result == 1;
}

// Inicializa o sensor BH1750
bool bh1750_init(bh1750_t *sensor, i2c_inst_t *i2c_port, uint8_t address) {
    sensor->i2c_port = i2c_port;
    sensor->address = address;
    
    // Power on
    if (!bh1750_send_cmd(sensor, BH1750_POWER_ON)) {
        return false;
    }
    sleep_ms(10);
    
    // Reset
    if (!bh1750_send_cmd(sensor, BH1750_RESET)) {
        return false;
    }
    sleep_ms(10);
    
    // Configurar modo de medição contínua de alta resolução
    if (!bh1750_send_cmd(sensor, BH1750_CONTINUOUS_HIGH_RES_MODE)) {
        return false;
    }
    sleep_ms(180); // Tempo para primeira medição
    
    return true;
}

// Lê o valor de luminosidade em lux
bool bh1750_read_light(bh1750_t *sensor, float *lux) {
    uint8_t data[2];
    
    // Lê 2 bytes do sensor
    int result = i2c_read_blocking(sensor->i2c_port, sensor->address, data, 2, false);
    
    if (result != 2) {
        return false;
    }
    
    // Combina os bytes e calcula o valor em lux
    uint16_t raw_value = (data[0] << 8) | data[1];
    *lux = raw_value / 1.2f;  // Fórmula de conversão para modo de alta resolução
    
    return true;
}

// Desliga o sensor (economia de energia)
void bh1750_power_down(bh1750_t *sensor) {
    bh1750_send_cmd(sensor, BH1750_POWER_DOWN);
}

// Liga o sensor
void bh1750_power_on(bh1750_t *sensor) {
    bh1750_send_cmd(sensor, BH1750_POWER_ON);
}
