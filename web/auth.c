#include "auth.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pico/stdlib.h"

#define AUTH_DEFAULT_USER "root"
#define AUTH_DEFAULT_PASS "root"

static char g_username[AUTH_USERNAME_MAX + 1];
static char g_password[AUTH_PASSWORD_MAX + 1];
static char g_session_token[AUTH_TOKEN_MAX + 1];
static bool g_session_valid = false;
static uint32_t g_session_counter = 0;

static void auth_generate_token(void) {
    uint32_t ms = to_ms_since_boot(get_absolute_time());
    g_session_counter++;
    snprintf(g_session_token, sizeof(g_session_token), "t%lu_%lu", (unsigned long)ms, (unsigned long)g_session_counter);
    g_session_valid = true;
}

static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void url_decode(const char *src, size_t len, char *dst, size_t dst_len) {
    size_t di = 0;
    for (size_t i = 0; i < len && di + 1 < dst_len; i++) {
        if (src[i] == '+') {
            dst[di++] = ' ';
        } else if (src[i] == '%' && i + 2 < len) {
            int hi = hex_to_int(src[i + 1]);
            int lo = hex_to_int(src[i + 2]);
            if (hi >= 0 && lo >= 0) {
                dst[di++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                dst[di++] = src[i];
            }
        } else {
            dst[di++] = src[i];
        }
    }
    dst[di] = '\0';
}

static bool get_form_value(const char *body, size_t len, const char *key, char *out, size_t out_len) {
    size_t key_len = strlen(key);
    size_t i = 0;

    while (i < len) {
        size_t seg_start = i;
        size_t seg_end = seg_start;
        while (seg_end < len && body[seg_end] != '&') {
            seg_end++;
        }

        size_t eq = seg_start;
        while (eq < seg_end && body[eq] != '=') {
            eq++;
        }

        if (eq < seg_end) {
            size_t klen = eq - seg_start;
            if (klen == key_len && strncmp(body + seg_start, key, key_len) == 0) {
                size_t vlen = seg_end - (eq + 1);
                url_decode(body + eq + 1, vlen, out, out_len);
                return true;
            }
        }

        if (seg_end >= len) break;
        i = seg_end + 1;
    }

    return false;
}

void auth_init(void) {
    strncpy(g_username, AUTH_DEFAULT_USER, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';
    strncpy(g_password, AUTH_DEFAULT_PASS, sizeof(g_password) - 1);
    g_password[sizeof(g_password) - 1] = '\0';
    g_session_valid = false;
    g_session_token[0] = '\0';
    g_session_counter = 0;
}

const char *auth_get_username(void) {
    return g_username;
}

bool auth_is_authenticated_request(const char *request) {
    if (!g_session_valid || g_session_token[0] == '\0') {
        return false;
    }

    const char *cookie = strstr(request, "Cookie:");
    if (!cookie) {
        return false;
    }

    const char *line_end = strstr(cookie, "\r\n");
    if (!line_end) {
        return false;
    }

    const char *session = strstr(cookie, "session=");
    if (!session || session > line_end) {
        return false;
    }

    session += strlen("session=");
    size_t token_len = 0;
    while (session + token_len < line_end && session[token_len] != ';' && session[token_len] != '\r') {
        token_len++;
    }

    if (token_len == 0 || token_len >= sizeof(g_session_token)) {
        return false;
    }

    return (strncmp(session, g_session_token, token_len) == 0 && g_session_token[token_len] == '\0');
}

bool auth_try_login(const char *body, size_t len, char *set_cookie_out, size_t out_len) {
    char username[AUTH_USERNAME_MAX + 1];
    char password[AUTH_PASSWORD_MAX + 1];

    if (!get_form_value(body, len, "username", username, sizeof(username)) ||
        !get_form_value(body, len, "password", password, sizeof(password))) {
        return false;
    }

    if (strcmp(username, g_username) != 0 || strcmp(password, g_password) != 0) {
        return false;
    }

    auth_generate_token();

    snprintf(set_cookie_out, out_len, "Set-Cookie: session=%s; Path=/; HttpOnly\r\n", g_session_token);
    return true;
}

void auth_logout(void) {
    g_session_valid = false;
    g_session_token[0] = '\0';
}

bool auth_update_credentials(const char *body, size_t len, char *message_out, size_t out_len) {
    char new_user[AUTH_USERNAME_MAX + 1];
    char new_pass[AUTH_PASSWORD_MAX + 1];

    if (!get_form_value(body, len, "new_user", new_user, sizeof(new_user)) ||
        !get_form_value(body, len, "new_pass", new_pass, sizeof(new_pass))) {
        snprintf(message_out, out_len, "Preencha usuario e senha.");
        return false;
    }

    if (new_user[0] == '\0' || new_pass[0] == '\0') {
        snprintf(message_out, out_len, "Usuario e senha nao podem ser vazios.");
        return false;
    }

    strncpy(g_username, new_user, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';
    strncpy(g_password, new_pass, sizeof(g_password) - 1);
    g_password[sizeof(g_password) - 1] = '\0';

    snprintf(message_out, out_len, "Credenciais atualizadas.");
    return true;
}

void auth_reset_credentials(void) {
    strncpy(g_username, AUTH_DEFAULT_USER, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';
    strncpy(g_password, AUTH_DEFAULT_PASS, sizeof(g_password) - 1);
    g_password[sizeof(g_password) - 1] = '\0';
    auth_logout();
}

bool auth_set_credentials(const char *user, const char *pass) {
    if (!user || !pass) {
        return false;
    }

    if (user[0] == '\0' || pass[0] == '\0') {
        return false;
    }

    strncpy(g_username, user, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';
    strncpy(g_password, pass, sizeof(g_password) - 1);
    g_password[sizeof(g_password) - 1] = '\0';

    return true;
}
