#include "aqualink_default_logger.h"

#include "logging.h"
#include "logging_levels.h"
#include "logging_error_handler.h"
#include "logging_formatter.h"
#include "logging_sink_console.h"
#include "logging_sink_basic_file.h"

const char AQUALINK_DEFAULT_LOGGER_NAME[] = "Aqualink Default Logger";

LoggingSink aqualink_default_logger_sink_file =
{
	.Config = 
	{ 
		.SinkIsInitialised = false
		//.SinkWriterMutex <-- cannot be inited this way.
	},

	.Initialise = &logging_sink_basic_file_initialise,
	.Formatter = &logging_sink_basic_file_formatter,
	.Pattern = &logging_sink_basic_file_pattern,
	.Writer = &logging_sink_basic_file_writer,
	.Flush = &logging_sink_basic_file_flush,
	.Close = &logging_sink_basic_file_close,
	
	.UserData = 0
};

LoggingSink aqualink_default_logging_sink_console =
{
	.Config =
	{
		.SinkIsInitialised = false
		//.SinkWriterMutex <-- cannot be inited this way.
	},

	.Initialise = &logging_sink_console_initialise,
	.Formatter = &logging_sink_console_formatter,
	.Pattern = &logging_sink_console_pattern,
	.Writer = &logging_sink_console_writer,
	.Flush = &logging_sink_console_flush,
	.Close = &logging_sink_console_close,

	.UserData = 0
};

LoggingSink_ListNode aqualink_default_logger_default_sinks =
{
	.sink = 0,
	.next = 0
};

Logger aqualink_default_logger =
{
	.Name = AQUALINK_DEFAULT_LOGGER_NAME,
	.Verbosity = Warning,
	.Sinks =
		{
			.head = 0,
			.tail = 0,
			.sink_count = 0
		},
	.ErrorHandler = &default_logger_error_handler,
	.Formatter = &default_logger_formatter
};
