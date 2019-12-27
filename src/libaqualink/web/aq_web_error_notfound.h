#ifndef AQ_WEB_ERROR_NOTFOUND_H_
#define AQ_WEB_ERROR_NOTFOUND_H_

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>

int aq_web_error_notfound(struct MHD_Connection* connection);

#endif // AQ_WEB_ERROR_NOTFOUND_H_
