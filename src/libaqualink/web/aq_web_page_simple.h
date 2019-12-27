#ifndef AQ_WEB_PAGE_SIMPLE_H_
#define AQ_WEB_PAGE_SIMPLE_H_

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <microhttpd.h>

int aq_web_page_simple(struct MHD_Connection* connection);

extern const char AQ_WEB_PAGE_SIMPLE_URL[];

#endif // AQ_WEB_PAGE_SIMPLE_H_
