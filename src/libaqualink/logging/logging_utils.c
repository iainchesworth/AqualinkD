#include "logging_utils.h"

#include <assert.h>
#include <string.h>

#include "string/string_utils.h"
#include "utils.h"

const char LOGGING_LEVEL_TRACE[] = "Trace";
const char LOGGING_LEVEL_DEBUG[] = "Debug";
const char LOGGING_LEVEL_INFO[] = "Info";
const char LOGGING_LEVEL_NOTICE[] = "Notice";
const char LOGGING_LEVEL_WARNING[] = "Warning";
const char LOGGING_LEVEL_ERROR[] = "Error";
const char LOGGING_LEVEL_CRITICAL[] = "Critical";
const char LOGGING_LEVEL_OFF[] = "Off";
const char LOGGING_LEVEL_UNKNOWN[] = "Unknown";

const char* logging_level_to_string(LoggingLevels level)
{
	switch (level)
	{
	case Trace: return LOGGING_LEVEL_TRACE;
	case Debug: return LOGGING_LEVEL_DEBUG;
	case Info: return LOGGING_LEVEL_INFO;
	case Notice: return LOGGING_LEVEL_NOTICE;
	case Warning: return LOGGING_LEVEL_WARNING;
	case Error: return LOGGING_LEVEL_ERROR;
	case Critical: return LOGGING_LEVEL_CRITICAL;
	case Off: return LOGGING_LEVEL_OFF;
	default: return LOGGING_LEVEL_UNKNOWN;
	}
}

LoggingLevels string_to_logging_level(const char* level)
{
	assert(0 != level);

	if (0 == strcmpi(LOGGING_LEVEL_TRACE, level)) { return Trace; }
	if (0 == strcmpi(LOGGING_LEVEL_DEBUG, level)) { return Debug; }
	if (0 == strcmpi(LOGGING_LEVEL_INFO, level)) { return Info; }
	if (0 == strcmpi(LOGGING_LEVEL_NOTICE, level)) { return Notice; }
	if (0 == strcmpi(LOGGING_LEVEL_WARNING, level)) { return Warning; }
	if (0 == strcmpi(LOGGING_LEVEL_ERROR, level)) { return Error; }
	if (0 == strcmpi(LOGGING_LEVEL_CRITICAL, level)) { return Critical; }
	
	// If there logging level is not identified, return "Off".
	return Off;
}
