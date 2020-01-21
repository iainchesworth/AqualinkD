#ifndef AQ_WEB_CONNECTION_HANDLER_H_
#define AQ_WEB_CONNECTION_HANDLER_H_

#include <libwebsockets.h>
#include <stdint.h>


int aq_web_connection_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

#endif // AQ_WEB_CONNECTION_HANDLER_H_
