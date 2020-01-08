#include "logging_sink_console.h"
#include "logging_sink_console_terminal.h"

#include <assert.h>
#include <stdio.h>

#include "logging.h"
#include "logging_levels.h"
#include "logging_sink.h"

void logging_sink_console_initialise(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		DEBUG("Console sink double initialisation...doing nothing this time");
	}
	else if (thrd_success != mtx_init(&(thisSink->Config.SinkWriterMutex), mtx_plain | mtx_recursive))
	{
		DEBUG("Console sink initialisation failed to create synchronisation mutex");
	}
	else
	{
		TRACE("Console sink initialisation completed successfully");
		thisSink->Config.SinkIsInitialised = true;
	}
}

void logging_sink_console_formatter(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		// Only do things if the sink is initialised.
	}
}

void logging_sink_console_pattern(LoggingSink* thisSink, const char* pattern)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		// Only do things if the sink is initialised.
	}
}

void logging_sink_console_writer(LoggingSink* thisSink, LoggingMessage message)
{
	assert(0 != thisSink);

	if (!thisSink->Config.SinkIsInitialised)
	{
		// Sink is not initialised...so do nothing.
	}
	else if (Off == message.Level)
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
	assert(0 != thisSink);
}

void logging_sink_console_close(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		TRACE("Destroying console sink synchronisation mutex");
		mtx_destroy(&(thisSink->Config.SinkWriterMutex));
	}
	else
	{
		DEBUG("Shutdown requested for a non-initialised console sink...doing nothing");
	}
}
