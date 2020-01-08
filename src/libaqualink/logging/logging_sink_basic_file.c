#include "logging_sink_basic_file.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "logging/logging.h"

void logging_sink_basic_file_initialise(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	// NOTE: append - Open file for output at the end of a file. Output operations always 
	// write data at the end of the file, expanding it. Repositioning operations (fseek, 
	// fsetpos, rewind) are ignored. The file is created if it does not exist.
	   
	LoggingSinkBasicFileUserData* fileSinkUserData = (LoggingSinkBasicFileUserData*)thisSink->UserData;
	static const char* LOGGING_FILE_MODE = "a";

	if (0 == fileSinkUserData)
	{
		WARN("No user data was configured for file sink...cannot continue");
	}
	else if (thisSink->Config.SinkIsInitialised)
	{
		DEBUG("File sink double initialisation...doing nothing this time");
	}
	else if (0 == fileSinkUserData->Filename)
	{
		DEBUG("No filename specified; not configuring file sink");
	}
	else if (0 == (fileSinkUserData->File = fopen(fileSinkUserData->Filename, LOGGING_FILE_MODE)))
	{
		ERROR("Failed to open specified log file: %s - error code %d", fileSinkUserData->Filename, errno);
	}
	else if (thrd_success != mtx_init(&(thisSink->Config.SinkWriterMutex), mtx_plain | mtx_recursive))
	{
		DEBUG("File sink initialisation failed to create synchronisation mutex");
	}
	else
	{
		TRACE("File sink initialisation completed successfully");
		thisSink->Config.SinkIsInitialised = true;
	}
}

void logging_sink_basic_file_formatter(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		// Only do things if the sink is initialised.
	}
}

void logging_sink_basic_file_pattern(LoggingSink* thisSink, const char* pattern)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		// Only do things if the sink is initialised.
	}
}

void logging_sink_basic_file_writer(LoggingSink* thisSink, LoggingMessage message)
{
	assert(0 != thisSink);

	if (thisSink->Config.SinkIsInitialised)
	{
		// Only do things if the sink is initialised.
	}
}

void logging_sink_basic_file_flush(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	LoggingSinkBasicFileUserData* fileSinkUserData = (LoggingSinkBasicFileUserData*)thisSink->UserData;

	if (!thisSink->Config.SinkIsInitialised)
	{
		// Sink is not initialised...so do nothing.
	}
	else if (0 == fileSinkUserData)
	{
		WARN("No user data was configured for file sink...cannot continue");
	}
	else if (0 != fileSinkUserData->File)
	{
		DEBUG("Output file is not open; not attempting to flush buffers");
	}
	else if (0 != fflush(fileSinkUserData->File))
	{
		ERROR("Failed to flush specified log file: %s - error code %d", fileSinkUserData->Filename, errno);
	}
	else
	{
		// Success...do nothing.
	}
}

void logging_sink_basic_file_close(LoggingSink* thisSink)
{
	assert(0 != thisSink);

	LoggingSinkBasicFileUserData* fileSinkUserData = (LoggingSinkBasicFileUserData*)thisSink->UserData;

	if (!thisSink->Config.SinkIsInitialised)
	{
		DEBUG("Shutdown requested for a non-initialised file sink...doing nothing");
	}
	else if (0 == fileSinkUserData)
	{
		WARN("No user data was configured for file sink...cannot continue");
	}
	else if (0 != fileSinkUserData->File)
	{
		DEBUG("Output file is not open; not attempting to close file");
	}
	else if (0 != fclose(fileSinkUserData->File))
	{
		ERROR("Failed to close specified log file: %s - error code %d", fileSinkUserData->Filename, errno);
	}
	else
	{
		DEBUG("Closed logging file: %s", fileSinkUserData->Filename);

		TRACE("Destroying file sink synchronisation mutex");
		mtx_destroy(&(thisSink->Config.SinkWriterMutex));
	}
}
