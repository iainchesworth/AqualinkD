#include "aq_web_error_notfound.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

int aq_web_error_notfound(struct MHD_Connection* connection)
{
    int ret;
    struct MHD_Response* response;
    const char* page = "<html><body>NOT FOUND</body></html>";

    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    if (!response)
    {
        return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}
