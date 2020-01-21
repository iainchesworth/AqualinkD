#include "aq_web_connection_handler.h"

#include <libwebsockets.h>

#include "aq_web_authentication.h"
#include "aq_web_http_api.h"
#include "aq_web_websockets.h"

#include "aqualink.h"

int aq_web_connection_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
	case LWS_CALLBACK_PROTOCOL_DESTROY:
	case LWS_CALLBACK_ESTABLISHED:
	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_SERVER_WRITEABLE:
		return aq_web_websockets_handler(wsi, reason, user, in, len);

	default:
		// There are many callbacks which we don't care about...ignore them here.
		break;
	}

	return aq_web_http_api_handler(wsi, reason, user, in, len);
}

