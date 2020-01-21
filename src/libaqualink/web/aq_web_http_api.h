#ifndef AQ_WEB_HTTP_API
#define AQ_WEB_HTTP_API

#include <libwebsockets.h>

int aq_web_http_api_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

#endif // AQ_WEB_HTTP_API
