#include "aq_web_page_simple.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

const char AQ_WEB_PAGE_SIMPLE_URL[] = "/simple.html";

int aq_web_page_simple(struct MHD_Connection* connection)
{
    int ret;
    struct MHD_Response* response;
    const char* page = "<html><body>SIMPLE</body></html>";

    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    if (!response)
    {
        return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
