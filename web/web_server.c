#include "web_server.h"
#include "sensor_data.h"
#include "auth.h"
#include "web_pages.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "lwip/err.h"

// Estado interno do servidor
static struct tcp_pcb *server_pcb = NULL;
static web_server_state_t server_state = WEB_SERVER_STOPPED;
static uint32_t request_count = 0;

// Buffer estático para construir respostas e requests
static char response_buffer[WEB_SERVER_BUFFER_SIZE];
static char request_buffer[WEB_SERVER_BUFFER_SIZE];

static int parse_request_line(const char *request, char *method_out, size_t method_len, char *path_out, size_t path_len) {
    const char *space = strchr(request, ' ');
    if (!space) return 0;

    size_t mlen = (size_t)(space - request);
    if (mlen + 1 > method_len) return 0;
    memcpy(method_out, request, mlen);
    method_out[mlen] = '\0';

    const char *path_start = space + 1;
    const char *path_end = strchr(path_start, ' ');
    if (!path_end) return 0;

    size_t plen = (size_t)(path_end - path_start);
    if (plen + 1 > path_len) return 0;
    memcpy(path_out, path_start, plen);
    path_out[plen] = '\0';

    char *query = strchr(path_out, '?');
    if (query) {
        *query = '\0';
    }

    return 1;
}

static const char *find_body(const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (!body) return NULL;
    return body + 4;
}

static void build_expire_cookie(char *buffer, size_t max_len) {
    snprintf(buffer, max_len, "Set-Cookie: session=; Path=/; Max-Age=0\r\n");
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
    
    int response_len = 0;

    // Copia request para buffer local com terminador
    size_t req_len = p->tot_len < (sizeof(request_buffer) - 1) ? p->tot_len : (sizeof(request_buffer) - 1);
    memcpy(request_buffer, p->payload, req_len);
    request_buffer[req_len] = '\0';

    char method[8];
    char path[64];
    if (parse_request_line(request_buffer, method, sizeof(method), path, sizeof(path)) == 0) {
        response_len = web_pages_generate_404(response_buffer, sizeof(response_buffer));
    } else {
        request_count++;

        const char *body = find_body(request_buffer);
        size_t body_len = 0;
        if (body && body >= request_buffer && body < request_buffer + req_len) {
            body_len = (size_t)(request_buffer + req_len - body);
        }

        bool is_authenticated = auth_is_authenticated_request(request_buffer);

        if (strcmp(method, "GET") == 0) {
            if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
                if (!is_authenticated) {
                    response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", NULL);
                } else {
                    sensor_data_t data = sensor_data_get();
                    response_len = web_pages_generate_dashboard(response_buffer, sizeof(response_buffer), &data);
                }
            } else if (strcmp(path, "/login") == 0) {
                response_len = web_pages_generate_login(response_buffer, sizeof(response_buffer), NULL);
            } else if (strcmp(path, "/logout") == 0) {
                char cookie_header[96];
                auth_logout();
                build_expire_cookie(cookie_header, sizeof(cookie_header));
                response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", cookie_header);
            } else if (strcmp(path, "/settings") == 0) {
                if (!is_authenticated) {
                    response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", NULL);
                } else {
                    response_len = web_pages_generate_settings(response_buffer, sizeof(response_buffer), NULL, auth_get_username());
                }
            } else if (strcmp(path, "/data") == 0) {
                if (!is_authenticated) {
                    response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", NULL);
                } else {
                    sensor_data_t data = sensor_data_get();
                    response_len = web_pages_generate_json(response_buffer, sizeof(response_buffer), &data);
                }
            } else {
                response_len = web_pages_generate_404(response_buffer, sizeof(response_buffer));
            }
        } else if (strcmp(method, "POST") == 0) {
            if (strcmp(path, "/login") == 0) {
                char set_cookie[96];
                if (auth_try_login(body ? body : "", body_len, set_cookie, sizeof(set_cookie))) {
                    response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/", set_cookie);
                } else {
                    response_len = web_pages_generate_login(response_buffer, sizeof(response_buffer), "Credenciais invalidas.");
                }
            } else if (strcmp(path, "/settings") == 0) {
                if (!is_authenticated) {
                    response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", NULL);
                } else {
                    char message[128];
                    message[0] = '\0';

                    if (body && strstr(body, "action=reset")) {
                        char cookie_header[96];
                        auth_reset_credentials();
                        build_expire_cookie(cookie_header, sizeof(cookie_header));
                        response_len = web_pages_generate_redirect(response_buffer, sizeof(response_buffer), "/login", cookie_header);
                    } else {
                        auth_update_credentials(body ? body : "", body_len, message, sizeof(message));
                        response_len = web_pages_generate_settings(response_buffer, sizeof(response_buffer), message, auth_get_username());
                    }
                }
            } else {
                response_len = web_pages_generate_404(response_buffer, sizeof(response_buffer));
            }
        } else {
            response_len = web_pages_generate_404(response_buffer, sizeof(response_buffer));
        }
    }

    // Envia resposta
    if (response_len > 0) {
        tcp_write(tpcb, response_buffer, response_len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
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
    auth_init();
    
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
