#ifndef AQ_LOGGING_H_
#define AQ_LOGGING_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "logging_error_handler.h"
#include "logging_formatter.h"
#include "logging_levels.h"
#include "logging_sink.h"
#include "logging_sink_registry.h"

#include <string.h>

typedef struct tagLogger
{
	const char* Name;
	LoggingLevels Verbosity;
	LoggingSinkRegistry Sinks;
	LoggingErrorHandler ErrorHandler;
	LoggingFormatter Formatter;
}
Logger;

#include "aqualink_default_logger.h"

void initialize_logging(Logger* logger);
void disable_backtrace(Logger* logger);
void enable_backtrace(Logger* logger, unsigned int messageCount);
void set_error_handler(Logger* logger, LoggingErrorHandler handler);
void set_pattern(Logger* logger, const char* pattern);
void set_verbosity(Logger* logger, LoggingLevels logLevel);
void shutdown_logging(Logger* logger);

void log_message(Logger* logger, LoggingLevels logLevel, const char file[], const char function[], const int line, const char fmt[], ...);

#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TRACE(...) log_message(&aqualink_default_logger, Trace, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define DEBUG(...) log_message(&aqualink_default_logger, Debug, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define INFO(...) log_message(&aqualink_default_logger, Info, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define NOTICE(...) log_message(&aqualink_default_logger, Notice, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define WARN(...) log_message(&aqualink_default_logger, Warning, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define ERROR(...) log_message(&aqualink_default_logger, Error, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define CRITICAL(...) log_message(&aqualink_default_logger, Critical, FILENAME, __func__, __LINE__, __VA_ARGS__)

#define TRACE_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Trace, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define DEBUG_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Debug, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define INFO_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Info, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define NOTICE_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Notice, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define WARN_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Warning, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define ERROR_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Error, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define CRITICAL_IF(cond, ...) { if (cond) { log_message(&aqualink_default_logger, Critical, FILENAME, __func__, __LINE__, __VA_ARGS__); } }

#define TRACE_TO(logger, ...) log_message(logger, Trace, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_TO(logger, ...) log_message(logger, Debug, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define INFO_TO(logger, ...) log_message(logger, Info, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define NOTICE_TO(logger, ...) log_message(logger, Notice, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define WARN_TO(logger, ...) log_message(logger, Warning, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define ERROR_TO(logger, ...) log_message(logger, Error, FILENAME, __func__, __LINE__, __VA_ARGS__)
#define CRITICAL_TO(logger, ...) log_message(logger, Critical, FILENAME, __func__, __LINE__, __VA_ARGS__)

#define TRACE_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Trace, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define DEBUG_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Debug, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define INFO_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Info, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define NOTICE_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Notice, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define WARN_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Warning, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define ERROR_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Error, FILENAME, __func__, __LINE__, __VA_ARGS__); } }
#define CRITICAL_TO_IF(cond, logger, ...) { if (cond) { log_message(logger, Critical, FILENAME, __func__, __LINE__, __VA_ARGS__); } }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_H_
