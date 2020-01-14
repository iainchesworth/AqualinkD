#include "aq_web_connection_handler.h"

#include <assert.h>
#include <microhttpd.h>

#include "logging/logging.h"
#include "utility/utils.h"

#include "aq_web.h"
#include "aq_web_error_methodnotallowed.h"
#include "aq_web_error_notfound.h"
#include "aq_web_page_controller.h"
#include "aq_web_page_simple.h"
#include "aq_web_websockets.h"

int aq_web_connection_handler(struct MHD_Connection* connection, conn_t* conn, const char* url)
{
	assert(0 != connection);
	assert(0 != conn);
	assert(0 != url);

	static const char UPGRADE_HEADER_KEY[] = "Upgrade";
	const char* upgrade_header_value = "";

	int ret = MHD_NO;

	switch (conn->method_id)
	{
	case WEBSOCKET:
		DEBUG("WEBSOCKET");
		{
		}
		break;

	case HTTP_GET:
		DEBUG("HTTP_GET");
		{
			if (0 == url)
			{
				WARN("No URL string provided...cannot return sensible response");
			}
			else if (0 != (upgrade_header_value = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, UPGRADE_HEADER_KEY)))
			{
				INFO("Upgrading connection to websockets");
				ret = handle_websocket_upgrade(connection);
			}
			else if (0 == strcmp(url, AQ_WEB_PAGE_SIMPLE_URL))
			{
				ret = aq_web_page_simple(connection);
			}
			else if (0 == strcmp(url, AQ_WEB_PAGE_SIMPLE_URL))
			{
				ret = aq_web_page_controller(connection);
			}
			else
			{
				// The client requested a url that we don't support, return the NOT FOUND error page.
				DEBUG("NOT FOUND (404) - Client requested a URL that does not exist");
				ret = aq_web_error_notfound(connection);
			}
		}
		break;

	case HTTP_PUT:
	case HTTP_DELETE:
	case HTTP_POST:
	default:
		DEBUG("METHOD NOT ALLOWED (405) - Client requested a HTTP method that is not supported");
		{
			// The client requested a method that we don't support, return the METHOD NOT ALLOWED error page.
			ret = aq_web_error_methodnotallowed(connection);
		}
		break;
	}

	return ret;
}
