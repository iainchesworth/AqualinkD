#include "aq_web_threaded.h"
#include "aq_web_authentication.h"
#include "aq_web_connection_handler.h"

#include <assert.h>
#include <libwebsockets.h>

#include "config/config_helpers.h"
#include "logging/logging.h"
#include "threads/thread_utils.h"

static const struct lws_protocols protocols[] =
{
	// first protocol must always be HTTP handler
	{
		.callback = lws_callback_http_dummy,
		.id = 1,
		.name = "http",
		.per_session_data_size = 0,
		.rx_buffer_size = 0,
		.tx_packet_size = 0,
		.user = 0
	},
	{
		.callback = aq_web_connection_handler,
		.id = 2,
		.name = "http-callbacks",
		.per_session_data_size = 0,
		.rx_buffer_size = 0,
		.tx_packet_size = 0,
		.user = 0
	},
	{
		/* TERMINATOR */
		.callback = 0,
		.id = 0,
		.name = 0,
		.per_session_data_size = 0,
		.rx_buffer_size = 0,
		.tx_packet_size = 0,
		.user = 0
	}
};

static const struct lws_http_mount mount = 
{
	.mount_next = 0,
	.mountpoint = "/",
	.origin = "./mount-origin",
	.def = "index.html",
	.protocol = 0,
	.cgienv = 0,
	.extra_mimetypes = 0,
	.interpret = 0,
	.cgi_timeout = 0,
	.cache_max_age = 0,
	.auth_mask = 0,
	.cache_reusable = 0,
	.cache_revalidate = 0,
	.cache_intermediaries = 0,
	.origin_protocol = LWSMPRO_FILE,
	.mountpoint_len = 1,
	.basic_auth_login_file = 0,
};

int webserver_thread(void* termination_handler_ptr)
{
	assert(0 != termination_handler_ptr);

	struct lws_context_creation_info info;
	struct lws_context* context;

	int returnCode = 0;

	memset(&info, 0, sizeof info);
	info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
	info.port = CFG_SocketPort();
	info.mounts = &mount;
	info.protocols = protocols;

	if (!CFG_Insecure())
	{
		DEBUG("Configuring webserver with support for TLS");
		info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		info.port = CFG_SslSocketPort();
		info.ssl_cert_filepath = load_pem_file("");
		info.ssl_private_key_filepath = load_pem_file("");
	}

	context = lws_create_context(&info);
	if (context == NULL)
	{
		ERROR("Failed to start web server");
		trigger_application_termination();
		returnCode = -1;
	}
	else
	{
		INFO("Running libwebsockets...%s", (!CFG_Insecure()) ? "with TLS support" : "");

		do
		{
			returnCode = lws_service(context, 0);
		} 
		while ((0 <= returnCode) && (!test_for_termination()));

		TRACE("Web server worker thread is stopping");
		lws_context_destroy(context);
	}

	return returnCode;
}
