#include "aq_web_websockets.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

static void upgrade_cb(void* cls, struct MHD_Connection* connection, void* con_cls, const char* extra_in, size_t extra_in_size, MHD_socket sock, struct MHD_UpgradeResponseHandle* urh)
{

}

int handle_websocket_upgrade(struct MHD_Connection* connection)
{
	struct MHD_Response* resp;
	int ret;

	resp = MHD_create_response_for_upgrade(&upgrade_cb, 0);
	MHD_add_response_header(resp, MHD_HTTP_HEADER_UPGRADE, "Hello World Protocol");
	ret = MHD_queue_response(connection, MHD_HTTP_SWITCHING_PROTOCOLS, resp);
	MHD_destroy_response(resp);

	return ret;
}
