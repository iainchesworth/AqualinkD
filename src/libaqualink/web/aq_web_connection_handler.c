#include "aq_web_connection_handler.h"

#include <assert.h>
#include <libwebsockets.h>
#include <stdint.h>

#include "logging/logging.h"
#include "utility/utils.h"

#include "aq_web_error_methodnotallowed.h"
#include "aq_web_error_notfound.h"
#include "aq_web_page_controller.h"
#include "aq_web_page_simple.h"
#include "aq_web_websockets.h"

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

int aq_web_connection_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	assert(0 != wsi);
	assert(0 != user);
	assert(0 != in);
	
	struct per_session_data* pss = (struct per_session_data*)user;
	struct per_vhost_data__minimal* vhd = (struct per_vhost_data__minimal*) lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
	
	int return_code = -1;

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
				return_code = 1;
			} 
			else  if (connect_client(vhd))
			{
				lws_timed_callback_vh_protocol(vhd->vhost, vhd->protocol, LWS_CALLBACK_USER, 1);
			}
			else
			{
				TRACE("Websocket client successfully connected");
			}
		}
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		TRACE("LWS_CALLBACK_PROTOCOL_DESTROY");
		{
			lws_ring_destroy(vhd->ring);
		}
		break;

	case LWS_CALLBACK_ESTABLISHED:
		TRACE("LWS_CALLBACK_ESTABLISHED");
		{
			lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
			pss->tail = lws_ring_get_oldest_tail(vhd->ring);
			pss->wsi = wsi;
		}
		break;

	case LWS_CALLBACK_CLOSED:
		TRACE("LWS_CALLBACK_CLOSED");
		{
			lws_ll_fwd_remove(struct per_session_data, pss_list, pss, vhd->pss_list);
		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		TRACE("LWS_CALLBACK_SERVER_WRITEABLE");
		{
			// Handle websocket connections
			return_code = 0;
		}
		break;

	case LWS_CALLBACK_HTTP:
		/*
		 * prepare and write http headers... with regards to content-
		 * length, there are three approaches:
		 *
		 *  - http/1.0 or connection:close: no need, but no pipelining
		 *  - http/1.1 or connected:keep-alive (keep-alive is default for 1.1): content-length required
		 *  - http/2: no need, LWS_WRITE_HTTP_FINAL closes the stream
		 *
		 * giving the api below LWS_ILLEGAL_HTTP_CONTENT_LEN instead of
		 * a content length forces the connection response headers to
		 * send back "connection: close", disabling keep-alive.
		 *
		 * If you know the final content-length, it's always OK to give
		 * it and keep-alive can work then if otherwise possible.  But
		 * often you don't know it and avoiding having to compute it
		 * at header-time makes life easier at the server.
		 */

		TRACE("LWS_CALLBACK_HTTP");
		{
			// Handle HTTP API connections
			return_code = 0;
		}
		break;

	case LWS_CALLBACK_HTTP_WRITEABLE:
		/*
		 * For http/1, it's possible to send a large buffer at once,
		 * but lws will malloc() up a temp buffer to hold any data
		 * that the kernel didn't accept in one go.  This is expensive
		 * in memory and cpu, so it's better to stage the creation of
		 * the data to be sent each time.
		 *
		 * For http/2, large data frames would block the whole
		 * connection, not just the stream and are not allowed.  Lws
		 * will call back on writable when the stream both has transmit
		 * credit and the round-robin fair access for sibling streams
		 * allows it.
		 *
		 * For http/2, we must send the last part with
		 * LWS_WRITE_HTTP_FINAL to close the stream representing
		 * this transaction.
		 */

		TRACE("LWS_CALLBACK_HTTP_WRITEABLE");
		{
			// Handle HTTP API connections
			return_code = 0;
		}
		break;

	default:
		// There are many callbacks which we don't care about...ignore them here.
		break;

	}

	return return_code;
}
