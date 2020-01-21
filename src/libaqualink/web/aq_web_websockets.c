#include "aq_web_websockets.h"

#include <assert.h>
#include <libwebsockets.h>
#include <stdlib.h>

#include "logging/logging.h"

#include "aqualink.h"

static void destroy_message(void* _msg)
{
	struct msg* msg = _msg;

	free(msg->payload);
	msg->payload = 0;
	msg->len = 0;
}

static int connect_client(struct per_vhost_data__minimal* vhd)
{
	vhd->i.context = vhd->context;
	vhd->i.port = 443;
	vhd->i.address = "localhost";
	vhd->i.path = "/";
	vhd->i.host = vhd->i.address;
	vhd->i.origin = vhd->i.address;
	vhd->i.ssl_connection = 1;

	vhd->i.protocol = "aqualinkd-protocol";
	vhd->i.local_protocol_name = "aqualinkd";
	vhd->i.pwsi = &vhd->client_wsi;

	return !lws_client_connect_via_info(&vhd->i);
}

int aq_web_websockets_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	UNREFERENCED_PARAMETER(len);
	UNREFERENCED_PARAMETER(in);

	assert(0 != wsi);

	struct per_session_data* pss = (struct per_session_data*)user;
	struct per_vhost_data__minimal* vhd = (struct per_vhost_data__minimal*) lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

	int return_code = 1;

	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		TRACE("LWS_CALLBACK_PROTOCOL_INIT");
		{
			vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data__minimal));
			vhd->context = lws_get_context(wsi);
			vhd->protocol = lws_get_protocol(wsi);
			vhd->vhost = lws_get_vhost(wsi);

			vhd->ring = lws_ring_create(sizeof(struct msg), 8, destroy_message);

			if (!vhd->ring)
			{
				WARN("Failed to create ring buffer to handle web socket connections");
			}
			else if (connect_client(vhd))
			{
				lws_timed_callback_vh_protocol(vhd->vhost, vhd->protocol, LWS_CALLBACK_USER, 1);
			}
			else
			{
				TRACE("Websocket client successfully connected");
				return_code = 0;
			}
		}
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		TRACE("LWS_CALLBACK_PROTOCOL_DESTROY");
		{
			lws_ring_destroy(vhd->ring);
			return_code = 0;
		}
		break;

	case LWS_CALLBACK_ESTABLISHED:
		TRACE("LWS_CALLBACK_ESTABLISHED");
		{
			lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
			if (0 == pss)
			{
				WARN("Attempted to dereference pss within LWS_CALLBACK_ESTABLISHED but it was NULL");
			}
			else
			{
				pss->tail = lws_ring_get_oldest_tail(vhd->ring);
				pss->wsi = wsi;

				return_code = 0;
			}
		}
		break;

	case LWS_CALLBACK_CLOSED:
		TRACE("LWS_CALLBACK_CLOSED");
		{
			lws_ll_fwd_remove(struct per_session_data, pss_list, pss, vhd->pss_list);
			return_code = 0;
		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		TRACE("LWS_CALLBACK_SERVER_WRITEABLE");
		{
			// Handle websocket connections
			return_code = 0;
		}
		break;

	default:
		ERROR("Invalid reason was passed to websocket handler - cannot handle connection stage");
		break;
	}

	return return_code;
}
