#ifndef AQ_WEB_WEBSOCKETS_H_
#define AQ_WEB_WEBSOCKETS_H_

#include <microhttpd.h>

int handle_websocket_upgrade(struct MHD_Connection* connection);

#endif // AQ_WEB_WEBSOCKETS_H_
