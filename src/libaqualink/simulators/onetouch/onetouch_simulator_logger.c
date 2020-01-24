#include "onetouch_simulator_logger.h"

#include <assert.h>
#include <stdlib.h>

#include "config/config_helpers.h"
#include "logging/logging.h"
#include "logging/logging_levels.h"
#include "logging/logging_error_handler.h"
#include "logging/logging_formatter.h"
#include "logging/logging_message.h"
#include "logging/logging_sink.h"
#include "logging/logging_sink_console.h"

#include "aqualink.h"

const char ONETOUCH_SIMULATOR_LOGGER_NAME[] = "OneTouch Simulator Data Logger";

LoggingSink onetouch_simulator_logger_console_sink =
{
	.Config =
	{
		.SinkIsInitialised = false
		//.SinkWriterMutex <-- cannot be inited this way.
	},

	.Initialise = &logging_sink_console_initialise,	// DEFAULT - Basic File Sink
	.Formatter = &logging_sink_console_formatter,	// DEFAULT - Basic File Sink
	.Pattern = &logging_sink_console_pattern,		// DEFAULT - Basic File Sink
	.Writer = &logging_sink_console_writer,			// DEFAULT - Basic File Sink
	.Flush = &logging_sink_console_flush,			// DEFAULT - Basic File Sink
	.Close = &logging_sink_console_close,			// DEFAULT - Basic File Sink

	.UserData = 0
};

Logger onetouch_simulator_logger =
{
	.Name = ONETOUCH_SIMULATOR_LOGGER_NAME,
	.Verbosity = Trace,
	.Sinks = { 0, 0, 0 },
	.ErrorHandler = &onetouch_simulator_logger_error_handler,
	.Formatter = &onetouch_simulator_logger_formatter
};

void onetouch_simulator_logger_initialise()
{
	register_logging_sink(&onetouch_simulator_logger.Sinks, &onetouch_simulator_logger_console_sink);
}

void onetouch_simulator_logger_error_handler(const char error_message[])
{
	UNREFERENCED_PARAMETER(error_message);

	///FIXME
}

void onetouch_simulator_logger_formatter(char buffer[], unsigned int buffer_length, LoggingMessage logMessage)
{
	UNREFERENCED_PARAMETER(buffer_length);

	assert(0 != buffer);

	switch (logMessage.Level)
	{
	case Trace:
	case Debug:
	case Info:
	case Notice:
	case Warning:
	case Error:
	case Critical:
	case Off:
	default:
	{

	}
	break;
	}
}
