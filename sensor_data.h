#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "pico/stdlib.h"
#include "led_matrix.h"

/**
 * @brief Estrutura que armazena todos os dados dos sensores
 * 
 * Esta estrutura centraliza os dados coletados pelos sensores
 * para serem consumidos pelo display e pelo web server.
 */
typedef struct {
    // Dados do sensor BH1750 (luminosidade)
    float luminosity_lux;
    bool luminosity_valid;
    
    // Dados do sensor AHT10 (temperatura e umidade)
    float temperature_c;
    float humidity_percent;
    bool temp_humidity_valid;
    
    // Estado da matriz de LEDs
    bool led_matrix_enabled;
    led_intensity_t led_intensity;
    
    // Timestamp da última atualização (em ms desde o boot)
    uint32_t last_update_ms;
} sensor_data_t;

/**
 * @brief Inicializa a estrutura de dados dos sensores
 */
void sensor_data_init(void);

/**
 * @brief Atualiza os dados dos sensores
 * @param data Ponteiro para estrutura com os novos dados
 */
void sensor_data_update(const sensor_data_t *data);

/**
 * @brief Obtém uma cópia dos dados atuais dos sensores
 * @return Cópia da estrutura de dados
 */
sensor_data_t sensor_data_get(void);

/**
 * @brief Atualiza apenas os dados de luminosidade
 * @param lux Valor da luminosidade em lux
 * @param valid Se a leitura é válida
 */
void sensor_data_set_luminosity(float lux, bool valid);

/**
 * @brief Atualiza apenas os dados de temperatura e umidade
 * @param temp Temperatura em graus Celsius
 * @param humidity Umidade em porcentagem
 * @param valid Se a leitura é válida
 */
void sensor_data_set_temp_humidity(float temp, float humidity, bool valid);

/**
 * @brief Atualiza o estado da matriz de LEDs
 * @param enabled Se a matriz está habilitada
 * @param intensity Nível de intensidade atual
 */
void sensor_data_set_led_state(bool enabled, led_intensity_t intensity);

#endif // SENSOR_DATA_H
