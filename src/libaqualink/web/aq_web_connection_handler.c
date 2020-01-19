#include "aq_web_connection_handler.h"

#include <assert.h>
#include <libwebsockets.h>

#include "logging/logging.h"
#include "utility/utils.h"

#include "aq_web_error_methodnotallowed.h"
#include "aq_web_error_notfound.h"
#include "aq_web_page_controller.h"
#include "aq_web_page_simple.h"
#include "aq_web_websockets.h"

#include "aqualink.h"

int aq_web_connection_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	UNREFERENCED_PARAMETER(user);
	UNREFERENCED_PARAMETER(len);

	assert(0 != wsi);

	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		TRACE("LWS_CALLBACK_PROTOCOL_INIT");
		{

		}
		break;

	case LWS_CALLBACK_ESTABLISHED:
		TRACE("LWS_CALLBACK_ESTABLISHED");
		{

		}
		break;

	case LWS_CALLBACK_CLOSED:
		TRACE("LWS_CALLBACK_CLOSED");
		{

		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		TRACE("LWS_CALLBACK_SERVER_WRITEABLE");
		{

		}
		break;

	case LWS_CALLBACK_RECEIVE:
		TRACE("LWS_CALLBACK_RECEIVE");
		{

		}
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		TRACE("LWS_CALLBACK_CLIENT_WRITEABLE");
		{
			void* universal_response = "Hello, World!";
			lws_write(wsi, universal_response, strlen(universal_response), LWS_WRITE_HTTP);
		}
		break;

	case LWS_CALLBACK_HTTP:
		TRACE("LWS_CALLBACK_HTTP");
		break;

	default:
		break;

	}

	return 0;
}
