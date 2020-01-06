#ifndef AQ_LOGGING_ERROR_HANDLER_H_
#define AQ_LOGGING_ERROR_HANDLER_H_

typedef void (*LoggingErrorHandler)(const char error_message[]);

void default_logger_error_handler(const char error_message[]);

#endif // AQ_LOGGING_ERROR_HANDLER_H_
