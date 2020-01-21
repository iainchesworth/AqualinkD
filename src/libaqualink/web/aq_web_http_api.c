#include "aq_web_http_api.h"

#include <assert.h>
#include <libwebsockets.h>

#include "logging/logging.h"

#include "aqualink.h"

int aq_web_http_api_handler(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	UNREFERENCED_PARAMETER(user);
	UNREFERENCED_PARAMETER(in);
	UNREFERENCED_PARAMETER(len);

	assert(0 != wsi);

	int return_code = 1;

	switch (reason)
	{
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
			lws_callback_on_writable(wsi);
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

	return lws_callback_http_dummy(wsi, reason, user, in, len);
}
