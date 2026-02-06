#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "pico/stdlib.h"

/**
 * @brief Estados possíveis da conexão WiFi
 */
typedef enum {
    WIFI_STATE_NOT_INITIALIZED,  // WiFi não inicializado
    WIFI_STATE_DISCONNECTED,     // Desconectado
    WIFI_STATE_CONNECTING,       // Tentando conectar
    WIFI_STATE_CONNECTED,        // Conectado com sucesso
    WIFI_STATE_ERROR             // Erro na conexão
} wifi_state_t;

/**
 * @brief Inicializa o módulo WiFi do Pico W
 * 
 * Deve ser chamado antes de qualquer outra função WiFi.
 * Inicializa o chip CYW43 e prepara para conexão.
 * 
 * @return true se inicializado com sucesso, false caso contrário
 */
bool wifi_manager_init(void);

/**
 * @brief Conecta a uma rede WiFi
 * 
 * Tenta conectar à rede especificada. Esta função é bloqueante
 * e aguarda até conectar ou timeout.
 * 
 * @param ssid Nome da rede WiFi
 * @param password Senha da rede WiFi
 * @param timeout_ms Timeout em milissegundos (0 = usar padrão)
 * @return true se conectou com sucesso, false caso contrário
 */
bool wifi_manager_connect(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * @brief Desconecta da rede WiFi atual
 */
void wifi_manager_disconnect(void);

/**
 * @brief Obtém o estado atual da conexão WiFi
 * @return Estado atual (wifi_state_t)
 */
wifi_state_t wifi_manager_get_state(void);

/**
 * @brief Obtém o endereço IP atribuído
 * 
 * Só é válido quando o estado é WIFI_STATE_CONNECTED.
 * 
 * @return String com o endereço IP (ex: "192.168.1.100")
 */
const char* wifi_manager_get_ip(void);

/**
 * @brief Obtém o nome da rede conectada
 * @return String com o SSID ou NULL se não conectado
 */
const char* wifi_manager_get_ssid(void);

/**
 * @brief Função de polling para manter a conexão WiFi
 * 
 * Deve ser chamada periodicamente no loop principal
 * quando usando o modo lwip_poll.
 */
void wifi_manager_poll(void);

/**
 * @brief Obtém a força do sinal WiFi (RSSI)
 * @return Valor RSSI em dBm (valores típicos: -30 a -90)
 */
int wifi_manager_get_rssi(void);

/**
 * @brief Verifica se está conectado ao WiFi
 * @return true se conectado, false caso contrário
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Obtém string descritiva do estado atual
 * @return String com descrição do estado
 */
const char* wifi_manager_get_state_string(void);

#endif // WIFI_MANAGER_H
