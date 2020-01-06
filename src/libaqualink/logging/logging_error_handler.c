#include "logging_error_handler.h"

#include <stdio.h>

void default_logger_error_handler(const char error_message[])
{
	fprintf(stderr, "%s\n", error_message);
}
