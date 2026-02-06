#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

/**
 * @brief Configurações de WiFi
 * 
 * IMPORTANTE: Este arquivo contém credenciais sensíveis.
 * Adicione wifi_config.h ao seu .gitignore para não
 * commitar suas credenciais no repositório!
 * 
 * Exemplo para .gitignore:
 *   wifi_config.h
 */

// ============================================
// CONFIGURE SUAS CREDENCIAIS WIFI AQUI
// ============================================

#define WIFI_SSID       "Asgard"      // Nome da sua rede WiFi
#define WIFI_PASSWORD   "ma199720@"     // Senha da sua rede WiFi

// ============================================
// CONFIGURAÇÕES DO SERVIDOR WEB
// ============================================

#define WEB_SERVER_PORT 80                   // Porta HTTP padrão

// Timeout de conexão WiFi em milissegundos
#define WIFI_CONNECT_TIMEOUT_MS 30000        // 30 segundos

// Intervalo de tentativa de reconexão em milissegundos
#define WIFI_RECONNECT_INTERVAL_MS 5000      // 5 segundos

#endif // WIFI_CONFIG_H
