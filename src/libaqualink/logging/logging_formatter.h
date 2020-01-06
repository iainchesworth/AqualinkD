#ifndef AQ_LOGGING_FORMATTER_H_
#define AQ_LOGGING_FORMATTER_H_

#include "logging_message.h"

typedef void (*LoggingFormatter)(char buffer[], unsigned int buffer_length, LoggingMessage logMessage);

void default_logger_formatter(char buffer[], unsigned int buffer_length, LoggingMessage logMessage);

#endif // AQ_LOGGING_FORMATTER_H_
