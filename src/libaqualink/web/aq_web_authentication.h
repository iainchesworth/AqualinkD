#ifndef AQ_WEB_AUTHENTICATION_H_
#define AQ_WEB_AUTHENTICATION_H_

int ask_for_authentication(void);
int is_authenticated(void);

char* load_pem_file(const char* pem_file);

#endif // AQ_WEB_AUTHENTICATION_H_
