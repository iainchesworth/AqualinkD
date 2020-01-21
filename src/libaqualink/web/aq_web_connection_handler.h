#ifndef AQ_WEB_CONNECTION_HANDLER_H_
#define AQ_WEB_CONNECTION_HANDLER_H_

#include <libwebsockets.h>
#include <stdint.h>

struct msg
{
	void* payload; /* is malloc'd */
	size_t len;
};

struct per_session_data
{
	struct per_session_data* pss_list;
	struct lws* wsi;
	uint32_t tail;
};

struct per_vhost_data__minimal 
{
	struct lws_context* context;
	struct lws_vhost* vhost;
	const struct lws_protocols* protocol;

	struct per_session_data* pss_list; // linked-list of live pss

	struct lws_ring* ring;						// ringbuffer holding unsent messages
	struct lws_client_connect_info i;
	struct lws* client_wsi;
};

int aq_web_connection_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

#endif // AQ_WEB_CONNECTION_HANDLER_H_
