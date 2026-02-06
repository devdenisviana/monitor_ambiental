#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Porta padrão do servidor HTTP
 */
#define WEB_SERVER_PORT 80

/**
 * @brief Timeout para conexões HTTP em segundos
 */
#define WEB_SERVER_TIMEOUT_S 5

/**
 * @brief Tamanho do buffer para requisições/respostas HTTP
 */
#define WEB_SERVER_BUFFER_SIZE 1024

/**
 * @brief Número máximo de conexões simultâneas
 */
#define WEB_SERVER_MAX_CONNECTIONS 2

/**
 * @brief Estado do servidor web
 */
typedef enum {
    WEB_SERVER_STOPPED,
    WEB_SERVER_RUNNING,
    WEB_SERVER_ERROR
} web_server_state_t;

/**
 * @brief Inicializa o servidor web
 * 
 * @param port Porta TCP para o servidor HTTP (geralmente 80)
 * @return true se iniciado com sucesso, false caso contrário
 */
bool web_server_init(uint16_t port);

/**
 * @brief Para o servidor web e libera recursos
 */
void web_server_deinit(void);

/**
 * @brief Verifica se o servidor está em execução
 * 
 * @return true se rodando, false caso contrário
 */
bool web_server_is_running(void);

/**
 * @brief Obtém o estado atual do servidor
 * 
 * @return Estado do servidor
 */
web_server_state_t web_server_get_state(void);

/**
 * @brief Processa conexões pendentes (deve ser chamado no loop)
 * 
 * Esta função é não-bloqueante e deve ser chamada periodicamente
 * para processar requisições HTTP.
 */
void web_server_poll(void);

/**
 * @brief Obtém o número de requisições processadas
 * 
 * @return Contador de requisições desde o início
 */
uint32_t web_server_get_request_count(void);

#endif // WEB_SERVER_H
