#include "aq_web.h"

#include <assert.h>
#include <microhttpd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "threads/thread_utils.h"
#include "aq_web_authentication.h"
#include "aq_web_connection_handler.h"
#include "utils.h"

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

	const unsigned int daemon_flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG | MHD_ALLOW_UPGRADE | MHD_USE_TCP_FASTOPEN | MHD_ALLOW_SUSPEND_RESUME;
	const unsigned int daemon_port = 80;

	char* key_pem = 0, * cert_pem = 0;
	bool use_ssl = false;

	struct MHD_Daemon* daemon;
	int returnCode = 0;

	if (use_ssl)
	{
		const unsigned int ssl_daemon_flags = daemon_flags | MHD_USE_SSL;
		const unsigned int ssl_daemon_port = daemon_port;

		// NOTE: load_pem_file() will "malloc" the buffer for the return buffer...remember to free it.

		if ((0 == (key_pem = load_pem_file(""))) || (0 == (cert_pem = load_pem_file(""))))
		{
			ERROR("Failed to load specified PEM certificates");
			returnCode = -1;
		}
		else if (0 == (daemon = MHD_start_daemon(ssl_daemon_flags, ssl_daemon_port, 0, 0, &request_started, 0, MHD_OPTION_NOTIFY_COMPLETED, &request_closed, MHD_OPTION_HTTPS_MEM_KEY, key_pem, MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END)))
		{
			ERROR("Failed to start web server with TLS support");
			returnCode = -1;
		} 
		else
		{
			INFO("Running libmicrohttpd (with TLS)...");
		}
	}
	else
	{
		if (0 == (daemon = MHD_start_daemon(daemon_flags, daemon_port, 0, 0, &request_started, 0, MHD_OPTION_NOTIFY_COMPLETED, &request_closed, MHD_OPTION_END)))
		{
			ERROR("Failed to start web server (no TLS support)");
			returnCode = -1;
		}
		else
		{
			INFO("Running libmicrohttpd...");
		}
	}

	// Block this thread until the service is terminated.
	if (!wait_for_termination())
	{
		ERROR("Failed when attempting to block-wait for termination handler");
		returnCode = -1;
	}
		
	MHD_quiesce_daemon(daemon);
	MHD_stop_daemon(daemon);
	
	if (0 != cert_pem) { free(cert_pem); }
	if (0 != key_pem) { free(key_pem); }

	return returnCode;
}
