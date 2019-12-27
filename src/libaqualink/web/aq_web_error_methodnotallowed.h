#ifndef AQ_WEB_ERROR_METHODNOTALLOWED_H_
#define AQ_WEB_ERROR_METHODNOTALLOWED_H_

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>

int aq_web_error_methodnotallowed(struct MHD_Connection* connection);

#endif // AQ_WEB_ERROR_METHODNOTALLOWED_H_
