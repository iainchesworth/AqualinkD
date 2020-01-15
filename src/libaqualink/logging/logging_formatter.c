#include "logging_formatter.h"
#include "logging_utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cross-platform/time.h"

void default_logger_formatter(char buffer[], unsigned int buffer_length, LoggingMessage logMessage)
{
	assert(0 != buffer);

	const unsigned int TIME_BUFFER_LEN = 24;
	char* time_buffer = (char*)malloc(TIME_BUFFER_LEN);

	if (0 == time_buffer)
	{
		// Failed to allocate memory...don't do anything here because we will recurse...and that's bad!
	}
	else
	{
		time_t now;
		time(&now);

		struct tm* local = localtime(&now);
		strftime(time_buffer, TIME_BUFFER_LEN, "%Y-%b-%d %T", local);

		switch (logMessage.Level)
		{
		case Trace:
			// Format: 2014-Oct-31 23:46:59 | Filename:line | Function | [info] Message
			snprintf(buffer, buffer_length, "%s | %s:%d | %s | [%s] %s",
				time_buffer,
				logMessage.Payload.File, logMessage.Payload.Line, logMessage.Payload.Function,
				logging_level_to_string(logMessage.Level), logMessage.Payload.Format);
			break;

		case Debug:
			// Format: 2014-Oct-31 23:46:59 | Function | [info] Message
			snprintf(buffer, buffer_length, "%s | %s | [%s] %s",
				time_buffer,
				logMessage.Payload.Function,
				logging_level_to_string(logMessage.Level), logMessage.Payload.Format);
			break;

		case Info:
		case Notice:
		case Warning:
		case Error:
		case Critical:
			// Format: 2014-Oct-31 23:46:59 | [info] Message
			snprintf(buffer, buffer_length, "%s | [%s] %s",
				time_buffer,
				logging_level_to_string(logMessage.Level), logMessage.Payload.Format);
			break;

		case Off:
		default:
			// Do nothing...coz the level is either unknown or Off.
			break;
		}

		if (0 != time_buffer)
		{
			free(time_buffer);
		}
	}
}
