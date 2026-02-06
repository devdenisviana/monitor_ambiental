#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#include <stddef.h>

#define AUTH_USERNAME_MAX 31
#define AUTH_PASSWORD_MAX 31
#define AUTH_TOKEN_MAX 63

void auth_init(void);
bool auth_is_authenticated_request(const char *request);
bool auth_try_login(const char *body, size_t len, char *set_cookie_out, size_t out_len);
void auth_logout(void);
bool auth_update_credentials(const char *body, size_t len, char *message_out, size_t out_len);
void auth_reset_credentials(void);
const char *auth_get_username(void);
bool auth_set_credentials(const char *user, const char *pass);

#endif // AUTH_H
