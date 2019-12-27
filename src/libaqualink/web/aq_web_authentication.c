#include "aq_web_authentication.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>
#include <stdlib.h>

static char* string_to_base64(const char* expected)
{
	///TODO
	return 0;
}

int ask_for_authentication(struct MHD_Connection* connection, const char* realm)
{
	int ret;
	struct MHD_Response* response;
	char* headervalue;
	const char* strbase = "Basic realm=";

	response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
	if (!response)
	{
		return MHD_NO;
	}

	headervalue = malloc(strlen(strbase) + strlen(realm) + 1);
	if (!headervalue)
	{
		return MHD_NO;
	}

	strcpy(headervalue, strbase);
	strcat(headervalue, realm);

	ret = MHD_add_response_header(response, "WWW-Authenticate", headervalue);
	free(headervalue);
	if (!ret)
	{
		MHD_destroy_response(response);
		return MHD_NO;
	}

	ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);

	MHD_destroy_response(response);

	return ret;
}

int is_authenticated(struct MHD_Connection* connection, const char* username, const char* password)
{
	const char* headervalue;
	char* expected_b64, * expected;
	const char* strbase = "Basic ";
	int authenticated;

	headervalue = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Authorization");
	if (NULL == headervalue)
	{
		return 0;
	}
	if (0 != strncmp(headervalue, strbase, strlen(strbase)))
	{
		return 0;
	}

	expected = malloc(strlen(username) + 1 + strlen(password) + 1);
	if (NULL == expected)
	{
		return 0;
	}

	strcpy(expected, username);
	strcat(expected, ":");
	strcat(expected, password);

	expected_b64 = string_to_base64(expected);
	free(expected);
	if (NULL == expected_b64)
	{
		return 0;
	}

	authenticated = (strcmp(headervalue + strlen(strbase), expected_b64) == 0);

	free(expected_b64);

	return authenticated;
}

char* load_pem_file(const char* pem_file)
{
	///TODO
	return 0;
}
