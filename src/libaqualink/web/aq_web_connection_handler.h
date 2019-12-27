#ifndef AQ_WEB_CONNECTION_HANDLER_H_
#define AQ_WEB_CONNECTION_HANDLER_H_

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>
#include "aq_web.h"

int aq_web_connection_handler(struct MHD_Connection* connection, conn_t* conn, const char* url);

#endif // AQ_WEB_CONNECTION_HANDLER_H_
