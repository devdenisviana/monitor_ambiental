#include "rtos_tasks.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pico/stdlib.h"
#include "sensor_data.h"
#include "wifi_manager.h"
#include "auth.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"

#define UART_CMD_MAX 96

static char uart_cmd_buffer[UART_CMD_MAX];
static size_t uart_cmd_len = 0;

static void uart_print_help(void) {
    printf("\nComandos UART:\n");
    printf("  HELP                - Lista comandos\n");
    printf("  STATUS              - Mostra sensores\n");
    printf("  WIFI?               - Mostra estado WiFi/IP\n");
    printf("  LED ON|OFF           - Liga/Desliga matriz\n");
    printf("  LOGIN RESET         - Reseta usuario/senha\n");
    printf("  LOGIN SET <u> <p>   - Define usuario/senha\n");
    fflush(stdout);
}

static int str_equals_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int str_starts_with_ignore_case(const char *text, const char *prefix) {
    while (*prefix) {
        if (*text == '\0') return 0;
        if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        text++;
        prefix++;
    }
    return 1;
}

static void uart_handle_command(const char *cmd_line,
                                led_matrix_t *led_matrix,
                                volatile bool *led_enabled) {
    if (!cmd_line || cmd_line[0] == '\0') return;

    char cmd[UART_CMD_MAX];
    strncpy(cmd, cmd_line, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';

    char *p = cmd;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') return;

    if (str_equals_ignore_case(p, "HELP")) {
        uart_print_help();
        return;
    }

    if (str_equals_ignore_case(p, "STATUS")) {
        sensor_data_t data = sensor_data_get();
        printf("TEMP=%.1fC HUM=%.1f%% LUX=%.1f LED=%s\n",
               data.temperature_c,
               data.humidity_percent,
               data.luminosity_lux,
               data.led_matrix_enabled ? "ON" : "OFF");
        fflush(stdout);
        return;
    }

    if (str_equals_ignore_case(p, "WIFI?")) {
        printf("WIFI=%s IP=%s\n",
               wifi_manager_get_state_string(),
               wifi_manager_get_ip());
        fflush(stdout);
        return;
    }

    if (str_starts_with_ignore_case(p, "LED ")) {
        const char *arg = p + 4;
        while (*arg == ' ' || *arg == '\t') arg++;
        if (str_equals_ignore_case(arg, "ON")) {
            *led_enabled = true;
            sensor_data_set_led_state(true, LED_INTENSITY_LOW);
            printf("LED=ON\n");
        } else if (str_equals_ignore_case(arg, "OFF")) {
            *led_enabled = false;
            led_matrix_clear(led_matrix);
            sensor_data_set_led_state(false, LED_INTENSITY_OFF);
            printf("LED=OFF\n");
        } else {
            printf("Uso: LED ON|OFF\n");
        }
        fflush(stdout);
        return;
    }

    if (str_equals_ignore_case(p, "LOGIN RESET")) {
        auth_reset_credentials();
        printf("LOGIN=RESET\n");
        fflush(stdout);
        return;
    }

    if (str_starts_with_ignore_case(p, "LOGIN SET ")) {
        char user[AUTH_USERNAME_MAX + 1];
        char pass[AUTH_PASSWORD_MAX + 1];
        const char *args = p + 10;
        while (*args == ' ' || *args == '\t') args++;

        if (sscanf(args, "%31s %31s", user, pass) == 2) {
            if (auth_set_credentials(user, pass)) {
                printf("LOGIN=SET user=%s\n", user);
            } else {
                printf("Erro ao definir credenciais\n");
            }
        } else {
            printf("Uso: LOGIN SET <usuario> <senha>\n");
        }
        fflush(stdout);
        return;
    }

    printf("Comando desconhecido. Digite HELP.\n");
    fflush(stdout);
}

static void uart_poll(led_matrix_t *led_matrix, volatile bool *led_enabled) {
    int ch = getchar_timeout_us(0);
    while (ch != PICO_ERROR_TIMEOUT) {
        if (ch == '\r' || ch == '\n') {
            if (uart_cmd_len > 0) {
                uart_cmd_buffer[uart_cmd_len] = '\0';
                uart_handle_command(uart_cmd_buffer, led_matrix, led_enabled);
                uart_cmd_len = 0;
            }
        } else if (ch == 8 || ch == 127) {
            if (uart_cmd_len > 0) {
                uart_cmd_len--;
            }
        } else if (uart_cmd_len + 1 < sizeof(uart_cmd_buffer)) {
            uart_cmd_buffer[uart_cmd_len++] = (char)ch;
        }

        ch = getchar_timeout_us(0);
    }
}

void task_uart(void *param) {
    const rtos_task_params_t *params = (const rtos_task_params_t *)param;
    const app_context_t *ctx = params ? params->ctx : NULL;

    if (!ctx || !ctx->led_matrix || !ctx->led_matrix_enabled) {
        vTaskDelete(NULL);
    }

    uart_print_help();

    while (true) {
        uart_poll(ctx->led_matrix, ctx->led_matrix_enabled);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
