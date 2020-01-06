#ifndef AQ_LOGGING_SINK_CONSOLE_H_
#define AQ_LOGGING_SINK_CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "logging_levels.h"
#include "logging_sink.h"

void logging_sink_console_initialise(LoggingSink* thisSink);
void logging_sink_console_formatter(LoggingSink* thisSink);
void logging_sink_console_pattern(LoggingSink* thisSink, const char* pattern);
void logging_sink_console_writer(LoggingSink* thisSink, LoggingMessage message);
void logging_sink_console_flush(LoggingSink* thisSink);
void logging_sink_console_close(LoggingSink* thisSink);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_CONSOLE_H_
