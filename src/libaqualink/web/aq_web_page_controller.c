#include "aq_web_page_controller.h"

#include <microhttpd.h>
#include <string.h>

const char AQ_WEB_PAGE_CONTROLLER_URL[] = "/controller.html";

int aq_web_page_controller(struct MHD_Connection* connection)
{
    int ret;
    struct MHD_Response* response;
    const char* page = "<html><body>CONTROLLER</body></html>";

    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    if (!response)
    {
        return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
