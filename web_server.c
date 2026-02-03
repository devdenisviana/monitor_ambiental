#include "web_server.h"
#include "sensor_data.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "lwip/err.h"

// Estado interno do servidor
static struct tcp_pcb *server_pcb = NULL;
static web_server_state_t server_state = WEB_SERVER_STOPPED;
static uint32_t request_count = 0;

// Buffer estático para construir respostas
static char response_buffer[WEB_SERVER_BUFFER_SIZE];

/**
 * @brief Gera o HTML da página principal
 */
static int generate_html(char *buffer, size_t max_size) {
    sensor_data_t data = sensor_data_get();
    
    const char *html_template = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<meta http-equiv='refresh' content='3'>"
        "<title>Monitor Ambiental</title>"
        "</head><body>"
        "<h2>Monitor Ambiental</h2>"
        "<p>Temperatura: <strong>%.1f C</strong></p>"
        "<p>Umidade: <strong>%.1f %%</strong></p>"
        "<p>Luminosidade: <strong>%.1f lux</strong></p>"
        "<p>Matriz de LEDs: <strong>%s</strong></p>"
        "<p>Atualiza a cada 3s.</p>"
        "</body></html>";
    
    return snprintf(buffer, max_size, html_template,
                    data.temperature_c,
                    data.humidity_percent,
                    data.luminosity_lux,
                    data.led_matrix_enabled ? "Ligado" : "Desligado");
}

/**
 * @brief Gera JSON com dados dos sensores
 */
static int generate_json(char *buffer, size_t max_size) {
    sensor_data_t data = sensor_data_get();
    
    const char *json_template = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"temp\":%.1f,\"humidity\":%.1f,\"lux\":%.1f,\"led\":%s,\"uptime\":%lu}";
    
    return snprintf(buffer, max_size, json_template,
                    data.temperature_c,
                    data.humidity_percent,
                    data.luminosity_lux,
                    data.led_matrix_enabled ? "true" : "false",
                    to_ms_since_boot(get_absolute_time()) / 1000);
}

/**
 * @brief Gera resposta 404 Not Found
 */
static int generate_404(char *buffer, size_t max_size) {
    const char *response = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "404 - Pagina nao encontrada";
    
    return snprintf(buffer, max_size, "%s", response);
}

/**
 * @brief Callback quando a conexão é fechada
 */
static err_t tcp_close_callback(void *arg, struct tcp_pcb *tpcb) {
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);
    tcp_close(tpcb);
    return ERR_OK;
}

/**
 * @brief Callback quando dados são recebidos
 */
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        // Conexão fechada pelo cliente
        tcp_close_callback(arg, tpcb);
        return ERR_OK;
    }
    
    // Marca dados como recebidos
    tcp_recved(tpcb, p->tot_len);
    
    // Extrai requisição HTTP
    char *request = (char *)p->payload;
    int response_len = 0;
    
    // Parse simples: verifica se é GET e qual a rota
    if (strncmp(request, "GET ", 4) == 0) {
        request_count++;
        
        if (strncmp(request + 4, "/ ", 2) == 0 || 
            strncmp(request + 4, "/index.html", 11) == 0) {
            // Página principal
            response_len = generate_html(response_buffer, sizeof(response_buffer));
        } else if (strncmp(request + 4, "/data ", 6) == 0) {
            // Endpoint JSON
            response_len = generate_json(response_buffer, sizeof(response_buffer));
        } else {
            // 404
            response_len = generate_404(response_buffer, sizeof(response_buffer));
        }
        
        // Envia resposta
        if (response_len > 0) {
            tcp_write(tpcb, response_buffer, response_len, TCP_WRITE_FLAG_COPY);
            tcp_output(tpcb);
        }
    }
    
    // Libera buffer
    pbuf_free(p);
    
    // Fecha conexão
    tcp_close_callback(arg, tpcb);
    
    return ERR_OK;
}

/**
 * @brief Callback quando nova conexão é aceita
 */
static err_t tcp_accept_callback(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }
    
    // Configura callbacks para essa conexão
    tcp_arg(client_pcb, NULL);
    tcp_recv(client_pcb, tcp_recv_callback);
    tcp_err(client_pcb, NULL);
    
    return ERR_OK;
}

// ============= API PÚBLICA =============

bool web_server_init(uint16_t port) {
    if (server_state == WEB_SERVER_RUNNING) {
        printf("[WEB] Servidor ja esta rodando\n");
        return true;
    }
    
    printf("[WEB] Iniciando servidor na porta %d...\n", port);
    
    // Cria PCB TCP
    server_pcb = tcp_new();
    if (!server_pcb) {
        printf("[WEB] ERRO: Falha ao criar PCB TCP\n");
        server_state = WEB_SERVER_ERROR;
        return false;
    }
    
    // Bind na porta
    err_t err = tcp_bind(server_pcb, IP_ADDR_ANY, port);
    if (err != ERR_OK) {
        printf("[WEB] ERRO: Falha ao fazer bind na porta %d (erro: %d)\n", port, err);
        tcp_close(server_pcb);
        server_pcb = NULL;
        server_state = WEB_SERVER_ERROR;
        return false;
    }
    
    // Coloca em modo listen
    server_pcb = tcp_listen(server_pcb);
    if (!server_pcb) {
        printf("[WEB] ERRO: Falha ao colocar em modo listen\n");
        server_state = WEB_SERVER_ERROR;
        return false;
    }
    
    // Define callback de accept
    tcp_accept(server_pcb, tcp_accept_callback);
    
    server_state = WEB_SERVER_RUNNING;
    printf("[WEB] Servidor HTTP iniciado com sucesso!\n");
    
    return true;
}

void web_server_deinit(void) {
    if (server_pcb) {
        tcp_close(server_pcb);
        server_pcb = NULL;
    }
    server_state = WEB_SERVER_STOPPED;
    printf("[WEB] Servidor parado\n");
}

bool web_server_is_running(void) {
    return (server_state == WEB_SERVER_RUNNING);
}

web_server_state_t web_server_get_state(void) {
    return server_state;
}

void web_server_poll(void) {
    // Com lwip_threadsafe_background, o polling é automático
    // Esta função fica vazia mas pode ser usada para estatísticas futuras
}

uint32_t web_server_get_request_count(void) {
    return request_count;
}
