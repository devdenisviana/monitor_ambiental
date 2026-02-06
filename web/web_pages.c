#include "web_pages.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

int web_pages_generate_dashboard(char *buffer, size_t max_size, const sensor_data_t *data) {
    const char *html_template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Monitor Ambiental</title>"
        "<style>"
        "body{font-family:Arial,Helvetica,sans-serif;margin:20px;}"
        "nav a{margin-right:12px;}"
        "</style>"
        "</head><body>"
        "<h2>Monitor Ambiental</h2>"
        "<p>Temperatura: <strong id='temp'>%.1f C</strong></p>"
        "<p>Umidade: <strong id='humidity'>%.1f %%</strong></p>"
        "<p>Luminosidade: <strong id='lux'>%.1f lux</strong></p>"
        "<p>Matriz de LEDs: <strong id='led'>%s</strong></p>"
        "<p>Atualiza a cada 0.5s.</p>"
        "<form method='GET' action='/logout' style='margin-top:12px;'>"
        "<button type='submit'>Sair</button>"
        "</form>"
        "<script>"
        "async function refreshData(){"
        "try{"
        "const res=await fetch('/data');"
        "if(!res.ok)return;"
        "const data=await res.json();"
        "document.getElementById('temp').textContent=data.temp.toFixed(1)+' C';"
        "document.getElementById('humidity').textContent=data.humidity.toFixed(1)+' %';"
        "document.getElementById('lux').textContent=data.lux.toFixed(1)+' lux';"
        "document.getElementById('led').textContent=data.led?'Ligado':'Desligado';"
        "}catch(e){}"
        "}"
        "setInterval(refreshData,500);"
        "</script>"
        "</body></html>";

    return snprintf(buffer, max_size, html_template,
                    data->temperature_c,
                    data->humidity_percent,
                    data->luminosity_lux,
                    data->led_matrix_enabled ? "Ligado" : "Desligado");
}

int web_pages_generate_json(char *buffer, size_t max_size, const sensor_data_t *data) {
    const char *json_template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"temp\":%.1f,\"humidity\":%.1f,\"lux\":%.1f,\"led\":%s,\"uptime\":%lu}";

    return snprintf(buffer, max_size, json_template,
                    data->temperature_c,
                    data->humidity_percent,
                    data->luminosity_lux,
                    data->led_matrix_enabled ? "true" : "false",
                    (unsigned long)(to_ms_since_boot(get_absolute_time()) / 1000));
}

int web_pages_generate_login(char *buffer, size_t max_size, const char *message) {
    const char *msg = (message && message[0]) ? message : "";
    const char *html_template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Login</title>"
        "<style>"
        "body{font-family:Arial,Helvetica,sans-serif;margin:20px;}"
        ".msg{color:#c00;margin-bottom:10px;}"
        "label{display:block;margin-top:8px;}"
        "</style>"
        "</head><body>"
        "<h2>Login</h2>"
        "%s"
        "<form method='POST' action='/login'>"
        "<label>Usuario</label>"
        "<input type='text' name='username' required>"
        "<label>Senha</label>"
        "<input type='password' name='password' required>"
        "<div style='margin-top:12px;'>"
        "<button type='submit'>Entrar</button>"
        "</div>"
        "</form>"
        "<hr>"
        "<form method='GET' action='/settings'>"
        "<button type='submit'>Alterar usuario e senha</button>"
        "</form>"
        "<form method='POST' action='/settings' style='margin-top:8px;'>"
        "<input type='hidden' name='action' value='reset'>"
        "<button type='submit'>Resetar para padrao</button>"
        "</form>"
        "</body></html>";

    char msg_block[160];
    if (msg[0]) {
        snprintf(msg_block, sizeof(msg_block), "<div class='msg'>%s</div>", msg);
    } else {
        msg_block[0] = '\0';
    }

    return snprintf(buffer, max_size, html_template, msg_block);
}

int web_pages_generate_settings(char *buffer, size_t max_size, const char *message, const char *current_user) {
    const char *msg = (message && message[0]) ? message : "";
    const char *user = (current_user && current_user[0]) ? current_user : "-";
    const char *html_template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Credenciais</title>"
        "<style>"
        "body{font-family:Arial,Helvetica,sans-serif;margin:20px;}"
        "nav a{margin-right:12px;}"
        ".msg{color:#0a0;margin-bottom:10px;}"
        ".err{color:#c00;margin-bottom:10px;}"
        "label{display:block;margin-top:8px;}"
        "</style>"
        "</head><body>"
        "<h2>Credenciais</h2>"
        "<nav>"
        "<a href='/'>Dashboard</a>"
        "<a href='/logout'>Sair</a>"
        "</nav>"
        "<p>Usuario atual: <strong>%s</strong></p>"
        "%s"
        "<form method='POST' action='/settings'>"
        "<input type='hidden' name='action' value='update'>"
        "<label>Novo usuario</label>"
        "<input type='text' name='new_user' required>"
        "<label>Nova senha</label>"
        "<input type='password' name='new_pass' required>"
        "<div style='margin-top:12px;'>"
        "<button type='submit'>Salvar</button>"
        "</div>"
        "</form>"
        "<hr>"
        "<form method='POST' action='/settings'>"
        "<input type='hidden' name='action' value='reset'>"
        "<button type='submit'>Resetar para padrao</button>"
        "</form>"
        "</body></html>";

    char msg_block[200];
    if (msg[0]) {
        snprintf(msg_block, sizeof(msg_block), "<div class='msg'>%s</div>", msg);
    } else {
        msg_block[0] = '\0';
    }

    return snprintf(buffer, max_size, html_template, user, msg_block);
}

int web_pages_generate_redirect(char *buffer, size_t max_size, const char *location, const char *extra_headers) {
    if (!extra_headers) {
        extra_headers = "";
    }

    const char *template =
        "HTTP/1.1 302 Found\r\n"
        "Location: %s\r\n"
        "%s"
        "Connection: close\r\n"
        "\r\n";

    return snprintf(buffer, max_size, template, location, extra_headers);
}

int web_pages_generate_404(char *buffer, size_t max_size) {
    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "404 - Pagina nao encontrada";

    return snprintf(buffer, max_size, "%s", response);
}
