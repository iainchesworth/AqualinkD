#ifndef AQ_LOGGING_SINK_H_
#define AQ_LOGGING_SINK_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging_message.h"

// Forward declarations
struct LoggingSink;

typedef void(*LOG_INIT)(struct LoggingSink* thisSink);
typedef void(*LOG_FORMAT)(struct LoggingSink* thisSink);
typedef void(*LOG_PATTERN)(struct LoggingSink* thisSink, const char* pattern);
typedef void(*LOG_WRITER)(struct LoggingSink* thisSink, LoggingMessage message);
typedef void(*LOG_FLUSH)(struct LoggingSink* thisSink);
typedef void(*LOG_CLOSE)(struct LoggingSink* thisSink);

struct LoggingSink
{
	struct
	{
		bool SinkIsInitialised;
		mtx_t SinkWriterMutex;
	}
	Config;

	LOG_INIT Initialise;
	LOG_FORMAT Formatter;
	LOG_PATTERN Pattern;
	LOG_WRITER Writer;
	LOG_FLUSH Flush;
	LOG_CLOSE Close;

	void* UserData;
};

typedef struct LoggingSink LoggingSink;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_H_
