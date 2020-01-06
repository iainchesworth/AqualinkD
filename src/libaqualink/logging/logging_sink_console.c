#include "logging_sink_console.h"
#include "logging_sink_console_terminal.h"

#include <assert.h>
#include <stdio.h>

#include "logging_levels.h"
#include "logging_sink.h"

void logging_sink_console_initialise(LoggingSink* thisSink)
{
}

void logging_sink_console_formatter(LoggingSink* thisSink)
{
}

void logging_sink_console_pattern(LoggingSink* thisSink, const char* pattern)
{
}

void logging_sink_console_writer(LoggingSink* thisSink, LoggingMessage message)
{
	if (Off == message.Level)
	{
		// Do nothing, logging level is set to Off.
	}
	else
	{
		const char* message_colour;
		FILE* output_location = stdout;

		switch (message.Level)
		{
		case Trace: 
			message_colour = terminal_light_gray();
			break;

		case Debug:
		case Info:
			message_colour = terminal_white();
			break;

		case Notice:
		case Warning:
			message_colour = terminal_yellow();
			break;

		case Error:
		case Critical:
			message_colour = terminal_red();
			output_location = stderr;
			break;

		case Off:
		default:
			break;
		}

		fprintf(output_location, "%s%s%s\n", message_colour, message.Message, terminal_reset());
	}
}

void logging_sink_console_flush(LoggingSink* thisSink)
{
}

void logging_sink_console_close(LoggingSink* thisSink)
{
}
