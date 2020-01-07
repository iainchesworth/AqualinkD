#ifndef AQ_WEB_AUTHENTICATION_H_
#define AQ_WEB_AUTHENTICATION_H_

#include <microhttpd.h>

int ask_for_authentication(struct MHD_Connection* connection, const char* realm);
int is_authenticated(struct MHD_Connection* connection, const char* username, const char* password);

char* load_pem_file(const char* pem_file);

#endif // AQ_WEB_AUTHENTICATION_H_
