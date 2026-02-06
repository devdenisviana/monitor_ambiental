#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <stddef.h>
#include "sensor_data.h"

int web_pages_generate_dashboard(char *buffer, size_t max_size, const sensor_data_t *data);
int web_pages_generate_json(char *buffer, size_t max_size, const sensor_data_t *data);
int web_pages_generate_login(char *buffer, size_t max_size, const char *message);
int web_pages_generate_settings(char *buffer, size_t max_size, const char *message, const char *current_user);
int web_pages_generate_redirect(char *buffer, size_t max_size, const char *location, const char *extra_headers);
int web_pages_generate_404(char *buffer, size_t max_size);

#endif // WEB_PAGES_H
