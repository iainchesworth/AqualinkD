#include "logging.h"
#include "logging_error_handler.h"
#include "logging_formatter.h"
#include "logging_sink_console.h"
#include "logging_sink_registry.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void initialize_logging(Logger* logger)
{
	assert(0 != logger);
}

void disable_backtrace(Logger* logger)
{
	assert(0 != logger);
}

void enable_backtrace(Logger* logger, unsigned int messageCount)
{
	assert(0 != logger);
}

void set_error_handler(Logger* logger, LoggingErrorHandler handler)
{
	assert(0 != logger);

	logger->ErrorHandler = handler;
}

void set_verbosity(Logger* logger, LoggingLevels logLevel)
{
	assert(0 != logger);

	logger->Verbosity = logLevel;
}

void set_pattern(Logger* logger, const char* pattern)
{
	assert(0 != logger);
}

void shutdown_logging(Logger* logger)
{
	assert(0 != logger);

	LoggingSink_ListNode* node = logger->Sinks.head;
	int sink_index;

	for (sink_index = 0; sink_index < logger->Sinks.sink_count; ++sink_index)
	{
		if ((0 != node) && (0 != node->sink))
		{
			node->sink->Flush(node->sink);
			node->sink->Close(node->sink);
			node = node->next;
		}
	}
}

void log_message(Logger* logger, LoggingLevels logLevel, const char file[], const char function[], const int line, const char fmt[], ...)
{
	assert(0 != logger);

	if (Off == logger->Verbosity)
	{
		// Do nothing because the logger's verbosity is set to "Off".
	}
	else if (logLevel > logger->Verbosity)
	{
		// Do nothing because the requested log level is higher i.e. noiser than 
		// the logger's verbosity level.
	}
	else
	{
		unsigned int MAX_MESSAGE_BUFFER_LEN = 1024, MAX_FORMAT_BUFFER_LEN = 256;
		char message_buffer[MAX_MESSAGE_BUFFER_LEN], format_buffer[MAX_FORMAT_BUFFER_LEN];

		memset(message_buffer, 0, MAX_MESSAGE_BUFFER_LEN);
		memset(format_buffer, 0, MAX_FORMAT_BUFFER_LEN);

		va_list args;
		va_start(args, fmt);

		LoggingMessage message;
		message.LoggerName = logger->Name;
		message.Level = logLevel;
		message.Payload.File = file;
		message.Payload.Function = function;
		message.Payload.Line = line;
		message.Payload.Format = fmt;
		message.Message = message_buffer;
		
		logger->Formatter(format_buffer, MAX_FORMAT_BUFFER_LEN, message);
		vsnprintf(message_buffer, MAX_MESSAGE_BUFFER_LEN, format_buffer, args);
	
		LoggingSink_ListNode* node = logger->Sinks.head;
		int sink_index;

		for (sink_index = 0; sink_index < logger->Sinks.sink_count; ++sink_index)
		{
			if ((0 != node) && (0 != node->sink))
			{
				mtx_t* sink_writer_mutex = &(node->sink->Config.SinkWriterMutex);

				// Lock the writer mutex...  Note that we don't check for errors because it would all go 
				// wrong (in an error handling case) as we would need to log a message (and get infinite 
				// recursion).
				mtx_lock(sink_writer_mutex);

				// Sink mutex is locked...write the message to the sink.
				node->sink->Writer(node->sink, message);

				// Unlock the writer mutex...  Note that we don't check for errors because it would all go 
				// wrong (in an error handling case) as we would need to log a message (and get infinite 
				// recursion).
				mtx_unlock(sink_writer_mutex);
				
				// Increment the pointer to the next sink.
				node = node->next;
			}
		}

		va_end(args);
	}
}
