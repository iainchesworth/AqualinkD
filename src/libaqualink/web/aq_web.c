#include "aq_web.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <threads.h>
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

	logMessage(LOG_DEBUG, "AQ_Web.c | answer_to_connection | Handling HTTP connection request: %s %s %s\n", version, method, url);

	conn_t* conn = 0;
	int ret;

	if (0 != *con_cls)
	{
		logMessage(LOG_DEBUG, "AQ_Web.c | answer_to_connection | Re-entering processing for existing connection\n", method);
		conn = (conn_t*)*con_cls;
		ret = aq_web_connection_handler(connection, conn, url);
	}
	else if (0 != strcmp(method, "GET"))
	{
		// PUT, POST, DELETE, ...
		logMessage(LOG_INFO, "AQ_Web.c | answer_to_connection | Bad HTTP request method: %s\n", method);
		ret = MHD_NO;
	}
	else if (0 == (conn = malloc(sizeof(conn_t))))
	{
		logMessage(LOG_INFO, "AQ_Web.c | answer_to_connection | Failed to allocate memory for the connection parameters\n", method);
		ret = MHD_NO;
	}
	else
	{
		logMessage(LOG_DEBUG, "AQ_Web.c | answer_to_connection | Entering processing for new connection\n", method);
				
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

	const unsigned int daemon_flags = MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG | MHD_ALLOW_UPGRADE | MHD_USE_TCP_FASTOPEN | MHD_ALLOW_SUSPEND_RESUME;
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
			logMessage(LOG_ERR, "AQ_Web.c | webserver_thread() | Failed to load specified PEM certificates\n");
			returnCode = -1;
		}
		else if (0 == (daemon = MHD_start_daemon(ssl_daemon_flags, ssl_daemon_port, 0, 0, &request_started, 0, MHD_OPTION_NOTIFY_COMPLETED, &request_closed, MHD_OPTION_HTTPS_MEM_KEY, key_pem, MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END)))
		{
			logMessage(LOG_ERR, "AQ_Web.c | webserver_thread() | Failed to start web server with TLS support\n");
			returnCode = -1;
		} 
		else
		{
			logMessage(LOG_INFO, "AQ_Web.c | webserver_thread() | Running libmicrohttpd (with TLS)...\n");
		}
	}
	else
	{
		if (0 == (daemon = MHD_start_daemon(daemon_flags, daemon_port, 0, 0, &request_started, 0, MHD_OPTION_NOTIFY_COMPLETED, &request_closed, MHD_OPTION_END)))
		{
			logMessage(LOG_ERR, "AQ_Web.c | webserver_thread() | Failed to start web server (no TLS support)\n");
			returnCode = -1;
		}
		else
		{
			logMessage(LOG_INFO, "AQ_Web.c | webserver_thread() | Running libmicrohttpd...\n");
		}
	}

	// Block this thread until the service is terminated.
	if (!wait_for_termination())
	{
		logMessage(LOG_ERR, "AQ_Web.c | webserver_thread() | Failed when attempting to block-wait for termination handler\n");
		returnCode = -1;
	}
		
	MHD_quiesce_daemon(daemon);
	MHD_stop_daemon(daemon);
	
	if (0 != cert_pem) { free(cert_pem); }
	if (0 != key_pem) { free(key_pem); }

	return returnCode;
}
