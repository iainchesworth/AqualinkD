#ifndef AQ_WEB_H_
#define AQ_WEB_H_

enum
{
	HTTP_GET,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_POST,
	WEBSOCKET
};

typedef struct conn
{
	int method_id;
}
conn_t;

int webserver_thread(void* termination_handler_ptr);

#endif // AQ_WEB_H_
