#include "wifi_manager.h"
#include "wifi_config.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Estado interno do WiFi
static wifi_state_t g_wifi_state = WIFI_STATE_NOT_INITIALIZED;
static char g_ip_address[16] = "0.0.0.0";
static char g_connected_ssid[33] = "";
static bool g_cyw43_initialized = false;

bool wifi_manager_init(void) {
    // Evita inicialização duplicada
    if (g_cyw43_initialized) {
        printf("[WIFI] Chip CYW43 ja inicializado\n");
        fflush(stdout);
        return true;
    }
    
    printf("[WIFI] Inicializando chip CYW43...\n");
    fflush(stdout);
    
    // Inicializa o chip CYW43 (WiFi do Pico W)
    if (cyw43_arch_init()) {
        printf("[WIFI] ERRO: Falha ao inicializar cyw43_arch!\n");
        fflush(stdout);
        g_wifi_state = WIFI_STATE_ERROR;
        return false;
    }
    
    g_cyw43_initialized = true;
    
    // Habilita o modo station (cliente WiFi)
    cyw43_arch_enable_sta_mode();
    
    g_wifi_state = WIFI_STATE_DISCONNECTED;
    printf("[WIFI] Chip CYW43 inicializado com sucesso!\n");
    fflush(stdout);
    
    return true;
}

bool wifi_manager_connect(const char *ssid, const char *password, uint32_t timeout_ms) {
    if (g_wifi_state == WIFI_STATE_NOT_INITIALIZED || !g_cyw43_initialized) {
        printf("[WIFI] ERRO: WiFi nao inicializado!\n");
        fflush(stdout);
        return false;
    }
    
    // Usa timeout padrão se não especificado
    if (timeout_ms == 0) {
        timeout_ms = WIFI_CONNECT_TIMEOUT_MS;
    }
    
    printf("[WIFI] Conectando a rede: %s\n", ssid);
    printf("[WIFI] Timeout: %lu ms\n", timeout_ms);
    fflush(stdout);
    
    g_wifi_state = WIFI_STATE_CONNECTING;
    
    // Tenta conectar à rede WiFi
    // CYW43_AUTH_WPA2_AES_PSK é o tipo de autenticação mais comum
    int result = cyw43_arch_wifi_connect_timeout_ms(
        ssid, 
        password, 
        CYW43_AUTH_WPA2_AES_PSK, 
        timeout_ms
    );
    
    if (result != 0) {
        printf("[WIFI] ERRO: Falha ao conectar (codigo: %d)\n", result);
        fflush(stdout);
        g_wifi_state = WIFI_STATE_ERROR;
        return false;
    }
    
    // Conexão bem sucedida - obtém o IP
    const ip4_addr_t *ip = netif_ip4_addr(netif_default);
    if (ip) {
        snprintf(g_ip_address, sizeof(g_ip_address), "%s", ip4addr_ntoa(ip));
    }
    
    // Salva o SSID conectado
    strncpy(g_connected_ssid, ssid, sizeof(g_connected_ssid) - 1);
    g_connected_ssid[sizeof(g_connected_ssid) - 1] = '\0';
    
    g_wifi_state = WIFI_STATE_CONNECTED;
    
    printf("[WIFI] Conectado com sucesso!\n");
    printf("[WIFI] Endereco IP: %s\n", g_ip_address);
    fflush(stdout);
    
    return true;
}

void wifi_manager_disconnect(void) {
    if (g_wifi_state == WIFI_STATE_CONNECTED) {
        printf("[WIFI] Desconectando...\n");
        fflush(stdout);
        
        cyw43_arch_disable_sta_mode();
        
        g_wifi_state = WIFI_STATE_DISCONNECTED;
        g_connected_ssid[0] = '\0';
        strcpy(g_ip_address, "0.0.0.0");
        
        printf("[WIFI] Desconectado.\n");
        fflush(stdout);
    }
}

wifi_state_t wifi_manager_get_state(void) {
    return g_wifi_state;
}

const char* wifi_manager_get_ip(void) {
    return g_ip_address;
}

const char* wifi_manager_get_ssid(void) {
    if (g_wifi_state == WIFI_STATE_CONNECTED) {
        return g_connected_ssid;
    }
    return NULL;
}

void wifi_manager_poll(void) {
    // Com pico_cyw43_arch_lwip_threadsafe_background não é necessário polling manual
    // O stack de rede roda em background automaticamente
}

int wifi_manager_get_rssi(void) {
    int32_t rssi = 0;
    if (g_wifi_state == WIFI_STATE_CONNECTED) {
        cyw43_wifi_get_rssi(&cyw43_state, &rssi);
    }
    return (int)rssi;
}

bool wifi_manager_is_connected(void) {
    return (g_wifi_state == WIFI_STATE_CONNECTED);
}

const char* wifi_manager_get_state_string(void) {
    switch (g_wifi_state) {
        case WIFI_STATE_NOT_INITIALIZED:
            return "Nao inicializado";
        case WIFI_STATE_DISCONNECTED:
            return "Desconectado";
        case WIFI_STATE_CONNECTING:
            return "Conectando...";
        case WIFI_STATE_CONNECTED:
            return "Conectado";
        case WIFI_STATE_ERROR:
            return "Erro";
        default:
            return "Desconhecido";
    }
}
