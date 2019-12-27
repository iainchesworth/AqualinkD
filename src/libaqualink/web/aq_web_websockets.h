#ifndef AQ_WEB_WEBSOCKETS_H_
#define AQ_WEB_WEBSOCKETS_H_

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>

int handle_websocket_upgrade(struct MHD_Connection* connection);

#endif // AQ_WEB_WEBSOCKETS_H_
