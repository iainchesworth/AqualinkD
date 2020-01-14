#include "aq_web.h"

#include <assert.h>
#include <microhttpd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/config_helpers.h"
#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "threads/thread_utils.h"
#include "utility/utils.h"

#include "aq_web_authentication.h"
#include "aq_web_connection_handler.h"

static void webserver_panichandler(void* cls, const char* file, unsigned int line, const char* reason)
{
	WARN("Web server panic'd - reason: %s", reason);
	DEBUG("   MHD file: %s", file);
	DEBUG("   MHD line: %d", line);
}

static int request_started(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls)
{
	assert(0 != connection);
	assert(0 != url);
	assert(0 != method);
	assert(0 != version);

	DEBUG("Handling HTTP connection request: %s %s %s", version, method, url);

	conn_t* conn = 0;
	int ret;

	if (0 != *con_cls)
	{
		DEBUG("Re-entering processing for existing connection", method);
		conn = (conn_t*)*con_cls;
		ret = aq_web_connection_handler(connection, conn, url);
	}
	else if (0 != strcmp(method, "GET"))
	{
		// PUT, POST, DELETE, ...
		INFO("Bad HTTP request method: %s", method);
		ret = MHD_NO;
	}
	else if (0 == (conn = (conn_t*)malloc(sizeof(conn_t))))
	{
		INFO("Failed to allocate memory for the connection parameters", method);
		ret = MHD_NO;
	}
	else
	{
		DEBUG("Entering processing for new connection", method);

		conn->method_id = HTTP_GET;

		ret = aq_web_connection_handler(connection, conn, url);
	}

	return ret;

	/*

	MHD_create_response_for_upgrade();
	MHD_add_response_header(resp, MHD_HTTP_HEADER_UPGRADE, "Hello World Protocol");
	ret = MHD_queue_response(connection, MHD_HTTP_SWITCHING_PROTOCOLS, resp);
	MHD_destroy_response(resp);

	if (!is_authenticated(connection, "", ""))
	{
		return ask_for_authentication(connection, "");
	}

	*/
}

static void request_closed(void* cls, struct MHD_Connection* connection, void** con_cls, enum MHD_RequestTerminationCode toe)
{
	conn_t* conn = (conn_t*)*con_cls;

	if (0 != conn) { free(conn); }

	*con_cls = NULL;
}

int webserver_thread(void* termination_handler_ptr)
{
	assert(0 != termination_handler_ptr);

	const unsigned int base_flags = MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD |  MHD_USE_DEBUG | MHD_ALLOW_UPGRADE | MHD_ALLOW_SUSPEND_RESUME;
	
	char* key_pem = 0, * cert_pem = 0;
	bool use_ssl;

	struct MHD_OptionItem options[] =
	{
		{.option = MHD_OPTION_NOTIFY_COMPLETED,  .value = (intptr_t)&request_closed, .ptr_value = 0 },
		{.option = MHD_OPTION_HTTPS_MEM_KEY,	 .value = (intptr_t)key_pem,		 .ptr_value = 0 },	// Set the MHD_OPTION_HTTPS_MEM_KEY (key_pem) option if SSL = true
		{.option = MHD_OPTION_HTTPS_MEM_CERT,	 .value = (intptr_t)cert_pem,		 .ptr_value = 0 },	// Set the MHD_OPTION_HTTPS_MEM_CERT (cert_pem) option if SSL = true
		{.option = MHD_OPTION_END,				 .value = 0,						 .ptr_value = 0}
	};

	TRACE("Web server worker thread is starting");

	// First - validate SSL configuration.
	if (CFG_Insecure())
	{
		/// DOWNGRADE SILENTLY
		memset(&options[1], 0, sizeof(struct MHD_OptionItem));
		memset(&options[2], 0, sizeof(struct MHD_OptionItem));

		options[1].option = MHD_OPTION_END;
		use_ssl = false;
	}
	else if (MHD_NO == MHD_is_feature_supported(MHD_FEATURE_SSL))
	{
		/// WARN + DOWNGRADE
		WARN("Web server library doesn't support TLS...downgrading to HTTP");

		memset(&options[1], 0, sizeof(struct MHD_OptionItem));
		memset(&options[2], 0, sizeof(struct MHD_OptionItem));

		options[1].option = MHD_OPTION_END; 
		use_ssl = false;
	}
	else if ((0 == (key_pem = load_pem_file(""))) || (0 == (cert_pem = load_pem_file(""))))
	{
		/// WARN + DOWNGRADE
		WARN("Failed to load specified PEM certificates...downgrading to HTTP");

		memset(&options[1], 0, sizeof(struct MHD_OptionItem));
		memset(&options[2], 0, sizeof(struct MHD_OptionItem));

		options[1].option = MHD_OPTION_END; 
		use_ssl = false;
	}
	else
	{
		DEBUG("Configuring webserver with support for TLS");
		use_ssl = true;
	}

	const unsigned int webserver_flags = (use_ssl) ? base_flags | MHD_USE_TLS : base_flags;
	const unsigned int webserver_port = (use_ssl) ? CFG_SslSocketPort() : CFG_SocketPort();

	// Configure the panic handler from whih we can get error information.
	MHD_set_panic_func(&webserver_panichandler, 0);

	struct MHD_Daemon* daemon = 0;
	int returnCode = 0;
	
	if (0 == (daemon = MHD_start_daemon(webserver_flags, webserver_port, 0, 0, &request_started, 0, MHD_OPTION_ARRAY, &options, MHD_OPTION_END)))
	{
		ERROR("Failed to start web server");
		trigger_application_termination();
		returnCode = -1;
	} 
	else
	{
		INFO("Running libmicrohttpd...%s", (use_ssl) ? "with TLS support" : "");
	}

	// Block this thread until the service is terminated.
	if (!wait_for_termination())
	{
		ERROR("Failed when attempting to block-wait for termination handler");
		returnCode = -1;
	}

	TRACE("Web server worker thread is stopping");
		
	MHD_quiesce_daemon(daemon);
	MHD_stop_daemon(daemon);
	
	if (0 != cert_pem) { free(cert_pem); }
	if (0 != key_pem) { free(key_pem); }

	return returnCode;
}
