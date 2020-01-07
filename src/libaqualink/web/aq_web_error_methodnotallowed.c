#include "aq_web_error_methodnotallowed.h"

#include <microhttpd.h>
#include <string.h>

int aq_web_error_methodnotallowed(struct MHD_Connection* connection)
{
    int ret;
    struct MHD_Response* response;
    const char* page = "<html><body>METHOD NOT ALLOWED</body></html>";

    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    if (!response)
    {
        return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
    MHD_destroy_response(response);

    return ret;
}
