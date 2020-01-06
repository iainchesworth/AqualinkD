#ifndef AQ_LOGGING_SINK_BASIC_FILE_H_
#define AQ_LOGGING_SINK_BASIC_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "logging_levels.h"
#include "logging_sink.h"

#include <stdio.h>

void logging_sink_basic_file_initialise(LoggingSink* thisSink);
void logging_sink_basic_file_formatter(LoggingSink* thisSink);
void logging_sink_basic_file_pattern(LoggingSink* thisSink, const char* pattern);
void logging_sink_basic_file_writer(LoggingSink* thisSink, LoggingMessage message);
void logging_sink_basic_file_flush(LoggingSink* thisSink);
void logging_sink_basic_file_close(LoggingSink* thisSink);

typedef struct tagLoggingSinkBasicFileUserData
{
	const char* Filename;
	FILE* File;
}
LoggingSinkBasicFileUserData;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_BASIC_FILE_H_
