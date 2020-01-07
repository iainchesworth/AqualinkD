#ifndef AQ_WEB_PAGE_CONTROLLER_H_
#define AQ_WEB_PAGE_CONTROLLER_H_

#include <microhttpd.h>

int aq_web_page_controller(struct MHD_Connection* connection);

extern const char AQ_WEB_PAGE_CONTROLLER_URL[];

#endif // AQ_WEB_PAGE_CONTROLLER_H_